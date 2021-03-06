#include <Mahdi/system.h>
//**********************************************************
String RUN_define_vars(String inst) {
    //=>init vars
    defvar vars_store[MAX_VAR_ALLOC_INSTRUCTIONS];
    StrList vars=0;
    Boolean is_static=false;
    //=>reset errcode
    EXP_clear_errcode();
    //=>simplify define vars
    uint32 vars_count=RUNKIT_simplify_define_vars(inst,&vars,&is_static);
    //=>check for errors
    if(vars_count==0 || EXP_check_errcode(BAD_DEFINE_VARS_ERRC)){
        return 0;
    }
    //=>analyzing define vars
    uint8 store_count=RUNKIT_defvars_analyzing(vars,vars_count,vars_store,false,false);
    //=>check for errors
    if(store_count==0 || EXP_check_errcode(BAD_DEFINE_VARS_ERRC)){
        return 0;
    }
    //=>add vars store to memory
    for (uint8 i = 0; i < store_count; i++) {
        printf("DEF_VAR:%s;;%s;;%s\n",vars_store[i].name_var,vars_store[i].type_var,vars_store[i].value_var);
        //=>register var in memory (default is public var)
        Longint ret1 = VM_set_var(entry_table.current, vars_store[i].name_var, vars_store[i].value_var, vars_store[i].type_var,is_static,false, true);
        if (ret1 == 0) {
            EXP_set_errcode(BAD_DEFINE_VARS_ERRC);
            return 0;
        }
    }
  
    //show_memory(0);
    return 0;
}
//**********************************************************
String RUN_normal_func_call(String inst){
    /**
        1- self.p1(this.p2(p3(4,"hi"),6+87)) ---OK---
        2- console.p1(pkg.p2(p21=9,p3(p31="j"))) ---OK---
        3- p1(fs.p2("hi"),pk.p3(7-5)[0]) ---OK---
    */
    //=>init vars
    String rest_inst = 0, buffer = 0, word = 0, index = 0, func_name = 0, pack_name = 0;
    // uint8 func_type = FUNC_TYPE_NORMAL;
    StrList parameters = 0;
    uint32 params_len = 0;
    Boolean is_string = false, is_par = false, is_point=false;
    int32 pars = 0, bras = 0, acos = 0, st_func = -1, en_func = -1, tmp1 = -1,pars_num = -1, bras_num = 0, en_pack = -1,count_ind = 0, equal_pos = -1;
    uint32 len = STR_length(inst);
    //=>iterate inst chars
    for (uint32 i = 0; i < len; i++) {
        Boolean no_buffer = false;
        //=>check is string
        if (inst[i] == '\"' && (i == 0 || inst[i - 1] != '\\')) {
        is_string = BOOL_switch(is_string);
        }
        //=>if is equal
        if (!is_string && inst[i] == '=' && equal_pos == -1) {
            equal_pos = i;
        }
        //=>count parenthesis
        if (!is_string) {
            if (inst[i] == '(')pars++;
            else if (inst[i] == ')')pars--;
            else if (inst[i] == '[')bras++;
            else if (inst[i] == ']')bras--;
            else if (inst[i] == '{')acos++;
            else if (inst[i] == '}')acos--;
        }
        //=>if is "." (pack name).(func_name)
        if (!is_string && i + 1 < len && inst[i] == '.' && word != 0 && RUNKIT_is_valid_name(word, true)&& en_func == -1 && acos == 0) {
            st_func = tmp1;
            STR_init(&pack_name,word);
            func_name = 0;
            is_par = false;
            is_point=true;
            en_pack = i;
            word = 0;
            continue;
        }
        //=>if is '(' (func_name)
        if (!is_string && inst[i] == '(' && word != 0 && RUNKIT_is_valid_name(word, false) && en_func == -1 && acos == 0) {
            //=>if pack_name not exist st_func is -1
            if (!is_point){
                st_func = tmp1;
                pack_name=0;
            }
            st_func = -1;
            is_point=false;
            STR_init(&func_name,word);
            pars_num = pars - 1;
            bras_num = bras;
            parameters = 0;
            params_len = 0;
            is_par = true;
            word = 0;
            STR_empty(&buffer);
            continue;
        }
        //=>get parameters
        if (!is_string && !is_point && (inst[i] == ',' || (inst[i] == ')' && pars == pars_num)) && is_par && acos == 0 && bras == bras_num) {
            buffer = STR_trim_space(buffer);
            if (buffer != 0)SLIST_append(&parameters, buffer, params_len++);

            if (inst[i] == ')' && pars == pars_num) {
                is_par = false;
                en_func = i + 1;
            }
            if (!is_par && inst[i] == ')') {
                //=>is func has index
                int32 br = 0;
                for (uint32 b = i + 1; b < len; b++) {
                    if (inst[b] != '[' && inst[b] != ' ' && br == 0) break;
                    else if (inst[b] == '[')br++;
                    else if (inst[b] == ']')br--;
                    index = CH_append(index, inst[b]);
                    en_func = b + 1;
                    if (br == 0)break;
                }
                index = STR_substring(index, 1, STR_length(index) - 1);
                break;
            }
            STR_empty(&buffer);
            word = 0;
            continue;
        }
        //=>append to buffer & word
        if (!no_buffer) {
            buffer = CH_append(buffer, inst[i]);
        }
        if (!is_string && (inst[i] == ' ' || inst[i] == '.' || inst[i] == ',' || inst[i] == '(' || inst[i] == ')' || inst[i] == '=' || CH_search(single_operators, inst[i],ChArraySize(single_operators))) && bras == 0) {
            count_ind = i;
            word = 0;
        } else {
            if (word == 0) tmp1 = i;
            word = CH_append(word, inst[i]);
        }
        //    printf("TTTT(%c):%s\n",inst[i], word);
    }
    //=>determine pack_name is not exist ('this' or 'self' keywords)
    if(pack_name==0){
        //=>if function call is in a package method
        if(entry_table.current.pin>0){
            STR_init(&pack_name,"this");
        }
        //=>if function call is outer all packages
        else{
            STR_init(&pack_name,"self");
        }
    }
    //=>calling functions
     printf("***FUNC_CALL(%s):\nPname:%s,Fname:%s,Params:%s[%i],Index:%s,(start:%i,end:%i),Return:%s\n",
            inst, pack_name, func_name,
            SLIST_print(parameters, params_len),
            params_len, index, st_func, en_func,
            STR_substring(inst, st_func, en_func));
    //  show_memory(0);
    //=>init calling function
    String ret_vars = RUN_init_calling_function(pack_name, func_name, parameters, params_len, index);
    //=>if occur an error
    if(ret_vars==0 && EXP_check_errcode(BAD_FUNC_CALL_ERRC)){
        return 0;
    }
    //=>is function call is alone :) like sum(5)
    if (st_func < 2) {
        return 0;
    }
    //=>replace return vars to inst
    if (en_func > -1 && st_func > -1) {
        rest_inst = STR_multi_append(STR_substring(inst, 0, st_func), ret_vars, STR_substring(inst, en_func, 0), 0, 0, 0);
    }
    //show_memory(0)
    //=>return rest instruction
    return rest_inst;
}

//**********************************************************
/**
 * get a function arguments and init for entering it then set its parameters and return its return_vars
 * @author madkne
 * @version 1.0
 * @param pname
 * @param fname
 * @param params
 * @param param_len
 * @param index
 * @return String
 */
String RUN_init_calling_function(String pname,String fname,StrList params,uint32 params_len,String index) {
    //=>init vars
    StrList vars_return = 0;
    uint32 vars_ret_len = 0;
    String ret_vars = 0;
    int8 ret0 = 0;
    StrList params_type = 0;
    StrList params_name=0;
    //=>split param name and param value and determine parameters type
    map *params_split=RUN_detect_type_name_func_params(&params, params_len);
    if(params_split==0 && EXP_check_errcode(BAD_FUNC_PARAMS_ERRC)){
        //TODO:error
        printf("run#56686745\n");
        EXP_set_errcode(BAD_FUNC_CALL_ERRC);
        return 0;
    }
    // printf("VVVVV:%s=>%s\n",SLIST_print(params,params_len),_map_print(params_split));
    //=>check if function call is built-in
    bifs builtin_func=_bifs_get_by_name(fname);
    if(builtin_func.func_name!=0){
        //=>if package name not 'self' or 'this' keywords
            if(!STR_equal(pname,"this") && !STR_equal(pname,"self")){
                //TODO:error
                printf("run#566867\n");
                EXP_set_errcode(BAD_FUNC_CALL_ERRC);
                return 0;
            }
            //=>call built-in function
            //TODO:
    }
    //=>check if function call is module
    uint32 modid=0;
    uint32 modpackid=MODULE_return_modpack_id(pname,&modid);
    if(modpackid>0){
        printf("called a module package...:%i,%i,%s\n",modid,modpackid,fname);
        uint32 vars_return_len=MODULE_call_function(modid,modpackid,fname,params_split,params,params_len,&vars_return);
        //=>if an error
        if(vars_ret_len==0 && EXP_check_errcode(INVALID_MODULE_FUNC_ERRC)){
            EXP_set_errcode(BAD_FUNC_CALL_ERRC);
            return 0;
        }
        //TODO:
    }
  //=>check if function call is a method or global function
  else{

      //TODO:
  }
  //--------------------record all registers
    //   fust s = {entry_table.cur_fid, entry_table.cur_fin, entry_table.cur_sid, entry_table.cur_order,
    //       entry_table.parent_fin};
    //   append_fust(s);
    //   //--------------------set new parent fin
    //   entry_table.parent_fin = entry_table.cur_fin;
    //   //--------------------init function parameters and run function instructions
    //   if (func_type == FUNC_TYPE_NORMAL)
    //     ret0 = set_function_parameters(fname, params, param_len);
    //   else
    //     ret0 = run_package_module_function(func_type, pname, fname, params, param_len);
    // //  printf("STOP:%s\n",fname);
    //   //--------------------analyzing ret0
    //   Boolean is_return = false;
    //   if (ret0 == FUNC_RET_BAD) {
    //     //TODO:
    //   }
    //     //if exist a built-in or module or package function
    //   else if (ret0 == FUNC_RET_BUILTIN || ret0 == FUNC_TYPE_MODULE || ret0 == FUNC_TYPE_PACKAGE) {
    //     is_return = true;
    //   } else if (ret0 == FUNC_RET_NORMAL) {

    //     entry_table.cur_order = 1;
    //     entry_table.cur_sid = 0;
    //     APP_CONTROLLER();
    //     is_return = true;
    //   }
    //   //TODO:
    //   //--------------------if returned values
    //   if (is_return) {
    //     long_int ret_po_id = RETURN_TMP_POINTER_ID;
    //     str_list rets = 0;
    //     longint_list rets_ind = 0;
    //     uint32 rets_ind_len = 0;
    //     uint32 rets_len = char_split(get_Mpoint(ret_po_id).data, ';', &rets, true);
    //     for (uint32 i = 0; i < rets_len; ++i) {
    //       longint_list_append(&rets_ind, rets_ind_len++, find_index_var_memory(str_to_long_int(rets[i])));
    //     }
    //     edit_Mpoint(ret_po_id, 0, 0, true, false);

    //     //--------------------analyzing call function index
    //     if (index != 0) {
    //       index = simplification_var_index(index);
    //       int32 real_ind = str_to_int32(index);
    //       if (real_ind >= rets_len) {
    //         exception_handler("array_index_overflow", __func__, str_from_int32(real_ind), str_from_int32(
    //             rets_len - 1));
    //       } else {
    //         str_init(&ret_vars, return_value_var_complete(rets_ind[real_ind]));
    //       }
    //     } else {
    //       for (uint32 i = 0; i < rets_len; ++i) {
    //         String vall = return_value_var_complete(rets_ind[i]);
    //         // printf ("#@@@@@@@:%s[%i]=>%s\n", rets[i],rets_ind[i], vall);
    //         ret_vars = str_append(ret_vars, vall);
    //         if (i + 1 < rets_len)ret_vars = char_append(ret_vars, ',');
    //       }
    //     }

    //   }
    //   //--------------------get last registers data
    //   fust lst = get_last_fust();
    //   //--------------------call garbage_collector(gc)
    //   if (is_return && lst.fin != entry_table.cur_fin) {
    //     garbage_collector('f');
    //   }
    //   //--------------------delete cole nodes
    //   for (;;) {
    //     if (entry_table.cole_end == 0)break;
    //     if (entry_table.cole_end->fin == entry_table.cur_fin) {
    //       pop_last_cole();
    //     } else break;
    //   }
    //   //--------------------delete lole nodes
    //   for (;;) {
    //     if (entry_table.lole_end == 0)break;
    //     if (entry_table.lole_end->fin == entry_table.cur_fin) {
    //       pop_last_lole();
    //     } else break;
    //   }
    //   //--------------------return to parent function
    //   //print_struct(PRINT_FUNCTIONS_STACK_ST);

    //   delete_last_fust();
    //   entry_table.cur_fid = lst.fid;
    //   entry_table.cur_fin = lst.fin;
    //   entry_table.cur_sid = lst.sid;
    //   entry_table.cur_order = lst.order;
    //   entry_table.parent_fin = lst.parent_fin;
    //show_memory(0)
    //fmt.Scanf("%s")
  return ret_vars;
}
//**********************************************************
/**
 * get function parameters, split its name and value(if exist) and detect parameters types and modify params like : msg="Hello" => {'msg':'val;string'}
 * if param is var:   var;[type];[varid];[index]
 * if param is value: val;[type]
 * @author madkne
 * @version 1.0
 * @param params : (pointer)
 * @param params_len
 * @return map*
 */
map* RUN_detect_type_name_func_params(StrList *params,uint32 params_len){
    //=>init vars
    uint32 ret_len = 0;
    map* start=0;
    map* end=0;
    //=>iterate params
    for (uint32 i = 0; i < params_len; i++) {
        //=>init vars
        String par=0;
        String param_name=0;
        //=>get current param
        STR_init(&par,(*params[i]));
        uint32 len = STR_length(par);
        //=>check if have param name
        if(CH_last_indexof(par,'=',len)!=-1){
            //=>init vars
            Boolean isstr=false;
            String buf=0;
            //=>iterate param chars to split name and value
            for (uint32 j = 0; j < len; j++){
                //=>if is string
                if (par[j] == '\"' && (j == 0 || par[j - 1] != '\\')) {
                    isstr = BOOL_switch(isstr);
                }
                //=>if is equal(=)
                if(!isstr && param_name==0 && buf!=0 && par[j]=='='){
                    STR_init(&param_name,buf);
                    buf=0;
                    continue;
                }
                //=>append to buf
                buf=CH_append(buf,par[j]);
            }
            //=>modify param value
            STR_init(&(*params[i]),buf);
            STR_init(&par,buf);
        }
        //=>if param is a variable
        if (RUNKIT_is_valid_name(par, true)) {
            //TODO:
            // uint8 state = '1';
            // String par_var = 0;
            // if (params[i][0] == '&') {
            // str_init(&par_var, str_substring(params[i], 1, 0));
            // state = '2';
            // } else str_init(&par_var, params[i]);
            // String state_s = str_multi_append(";", char_to_str(state), ";", 0, 0, 0);
            // String name = 0, index = 0;
            // return_name_index_var(par_var, true, &name, &index);
            // long_int var_id = return_var_id(name, "0");
            // if (var_id > 0) {
            // long_int real_id = find_index_var_memory(var_id);
            // Mvar var = get_Mvar(real_id);
            // String type_name = 0;
            // str_init(&type_name, get_datas(var.type_var).name);
            // index = simplification_var_index(index);
            // //if is complete var
            // if (index == 0) str_init(&index, "_");
            // str_list_append(&(*ret),
            //                 str_multi_append(type_name, ";", str_from_long_int(real_id), state_s, index, 0), ret_len++);
            // continue;
            // }
        }
        //=>if param is a value
        else{
            //=>if param is a basic type value (string,number,boolean)
            uint8 subtype='0';
            String type=RUNKIT_detect_basic_type_value(par,&subtype);
            //=>if not a basic type value
            if(type==0 && EXP_check_errcode(INVALID_VALUE_ERRC)){
                //=>check for package types
                //TODO:
            }
            //=>append to map
            _map_push(&start,&end,(param_name==0)?STR_from_int32(i):param_name,STR_append("val;",type));
        }
    }
    return start;
}
//**********************************************************
/**
 * get src function parameters and check them with dest function parameters than want to call
 * @author madkne
 * @version 1.0
 * @since 2020.1.13
 */
Boolean RUN_check_func_params(map *Sparams_type,StrList Sparams,uint32 Sparams_len,StrList Dparams_name,StrList Dparams_type,StrList Dparams_def,uint32 Dparams_len){
    //=>init vars
    Boolean is_name=false;
    StrList used_params_name=0;
    uint32 used_params_name_len=0;
    //=>if Sparams len bigger than Dparams len
    if(Sparams_len>Dparams_len) return false;
    //=>iterate src params
    for (uint32 i = 0; i < Sparams_len; i++){
        //=>get src param type
        map param_type=_map_index(Sparams_type,i);
        //=>get src param type
        StrList param_type_ind=0;
        CH_split(param_type.value,';',&param_type_ind,true);
        //=>if param name not exist (in order)
        if(STR_is_num(param_type.key)){
            //=>if before used param name, not order
            if(is_name) return false;
            //=>check for src type and dest param type
            if(!STR_equal(param_type_ind[1],Dparams_type[i])) return false;
            //=>append to used params name
            SLIST_append(&used_params_name,Dparams_name[i],used_params_name_len++);
        }
        //=>if param name is exist (not order)
        else{
            is_name=true;
            //=>check not used before
            if(SLIST_search(used_params_name,used_params_name_len,param_type.key)!=-1) 
                return false;
            //=>get index by param name
            int32 ind=SLIST_search(Dparams_name,Dparams_len,param_type.key);
            //=>check if exist this param name
            if(ind==-1) return false;
            //=>check for src type and dest param type
            if(!STR_equal(param_type_ind[1],Dparams_type[ind])) return false;
            //=>append to used params name
            SLIST_append(&used_params_name,Dparams_name[ind],used_params_name_len++);
        }
    }
    //=>check if all required dest params is used
    for (uint32 i = 0; i < Dparams_len; i++){
        if(Dparams_def[i]==0 && SLIST_search(used_params_name,used_params_name_len,Dparams_name[i])==-1) return false;
    }
    return true;
}

//**********************************************************
/**
 * get function params with their types and convert them to values if is a variable and return a map: {param_name:param_value}
 * @author madkne
 * @version 1.0
 * @since 2020.1.13
 */
map* RUN_func_params_to_values(map *params_type,StrList params,uint32 params_len) {
    //=>init vars
    map *start=0;
    map *end=0;
    
    //=>iterate params
    for (uint32 i = 0; i < params_len; i++) {
        //=>get current param
        map param=_map_index(params_type,i);
         printf("param:%s=>%s\n",param.value,params[i]);
        //=>split params type
        StrList p1 = 0;
        CH_split(param.value, ';', &p1, false);
        //=>if param is var
        if (STR_equal(p1[0],"var")) {
            //TODO:
            // String var_val = 0;
            // //      printf("###argv%i:%s(VAR)%s\n", i, params[i], partypes[i]);
            // //=>if var not has any index
            // if (str_ch_equal(p1[3], '_')) var_val = return_value_var_complete(str_to_long_int(p1[1]));
            // //=> else if var has index
            // else {
            // long_int po_id = get_Mvar(str_to_long_int(p1[1])).pointer_id;
            // long_int data_id = get_data_memory_id(po_id, p1[3]);
            // var_val = __return_value_var_complete(data_id);
            // }
            // if (str_ch_equal(p1[3], '_') || str_ch_equal(p1[3], '1') && var_val != 0 && var_val[0] != '{')
            // var_val = str_reomve_quotations(var_val, p1[0]);
            // //=>if string is utf8
            // if (p1[0][0] == 's' && str_indexof(var_val, UTF8_ID_LABEL, 0) == 0) {
            // var_val = utf8_to_bytes_string(get_utst_by_label(var_val).utf8_string);
            // }
            // str_list_append(&(*argvs), var_val, i);
            // //if param[0] is var, store its index
            // if (i == 0) (*var0_ind) = str_to_long_int(p1[1]);
        }
        //=>if param is value
        else {
            //=>simplify value
            String ret1 = 0;
            ret1 = RUNKIT_calculate_value(params[i], p1[1],0,true);
            //=>if is an error
            if(ret1==0 && EXP_check_errcode(INVALID_VALUE_ERRC)){
                EXP_set_errcode(BAD_FUNC_PARAMS_ERRC);
                return 0;
            }
            //=>if string is utf8
            if (p1[1][0] == 's' && STR_indexof(ret1, UTF8_ID_LABEL, 0) == 0) {
                ret1 = USTR_to_bytes_str(_utst_get_by_label(ret1).utf8_string);
            } 
            //=>if string, remove quotations
            else{
                ret1 = STR_reomve_quotations(ret1, p1[1]);
            }
            printf("argvs_val:%s(%s)\n",ret1,p1[1]);
            //=>append to map
            _map_push(&start,&end,param.key,ret1);
        }
    }
    return start;
}

