#!/usr/bin/env python3

import argparse
import pickle
import json
import _dynStruct
import pyprind
import capstone

def get_args():
    parser = argparse.ArgumentParser(description='Dynstruct analize tool')
    parser.add_argument('-d', type=str, dest='dynamo_file',
                        help='output file from dynStruct dynamoRio client')
    parser.add_argument('-p', type=str, dest='previous_file',
                        help='file to load serialized data')
    parser.add_argument('-o', type=str, dest='out_pickle',
                        help='file to store serialized data.')
    parser.add_argument('-n', dest='run_recovering', action='store_false',
                        help='just load json without recovering structures')
    parser.add_argument('-k', dest='run_cleaning', action='store_false',
                        help='don\'t remove block which look like array after structure recovery')
    parser.add_argument('-e', type=str, default=None, dest='out_file',
                        metavar='<file_name>',
                        help='export structures in C style on <file_name>')
    parser.add_argument('-c', action='store_true', dest='console',
                        help='print structures in C style on console')
    parser.add_argument('-w', action='store_true', dest='web_view',
                        help='start the web view')
    parser.add_argument('-l', dest='bind_addr', default='127.0.0.1:24242', type=str,
                        help='bind addr for the web view default 127.0.0.1:24242')

    return parser.parse_args()



def load_json(json_data, l_block, l_access_w, l_access_r):
    id_block = 0

    if json_data["is_64"] == 1:
        _dynStruct.disasm = capstone.Cs(capstone.CS_ARCH_X86,
                                        capstone.CS_MODE_64)
    else:
        _dynStruct.disasm = capstone.Cs(capstone.CS_ARCH_X86,
                                        capstone.CS_MODE_32)
    # active detail to operand information (used to analize context instr)
    _dynStruct.disasm.detail = True
    try:
        prbar = pyprind.ProgBar(len(json_data["blocks"]), track_time=False, title="Loading Json data")
        for block in filter(None, json_data["blocks"]):
            l_block.append(_dynStruct.Block(block, l_access_w, l_access_r, id_block))
            id_block += 1
            prbar.update()
    except KeyError as e:
        print("Json not from dynamoRIO client, missing : %s" % str(e))
        return False
    return True

def main():
    args = get_args()

    if args.out_pickle:
        _dynStruct.serialized_file = args.out_pickle
    elif args.previous_file:
        _dynStruct.serialized_file = args.previous_file
        
    if not args.previous_file and not args.dynamo_file:
        print("Use -d or -p option to give\
 data extract by the gatherer or serialized data from a previous run")
        exit(0)
        
    if args.dynamo_file:
        f = open(args.dynamo_file, "r")
        json_data = json.load(f)
        f.close()
        load_json(json_data, _dynStruct.l_block, _dynStruct.l_access_w, _dynStruct.l_access_r)
        if args.run_recovering:
            _dynStruct.Struct.recover_all_struct(_dynStruct.l_block, _dynStruct.l_struct)
            _dynStruct.Struct.clean_all_struct(_dynStruct.l_struct, args.run_cleaning)

        # We don't need capstone instr object anymore sot remove them
        _dynStruct.Access.remove_instrs(_dynStruct.l_access_w)
        _dynStruct.Access.remove_instrs(_dynStruct.l_access_r)
    elif args.previous_file:
        with open(args.previous_file, "rb") as f:
            data = pickle.load(f)
        _dynStruct.l_struct = data["structs"]
        _dynStruct.l_block = data["blocks"]
        _dynStruct.l_access_w = data["w_access"]
        _dynStruct.l_access_r = data["r_access"]
        
    if args.out_pickle:
        _dynStruct.save_pickle(args.out_pickle, _dynStruct.l_struct, _dynStruct.l_block,
                               _dynStruct.l_access_w, _dynStruct.l_access_r)

    if args.out_file:
        _dynStruct.print_to_file(args.out_file, _dynStruct.l_struct)

    if args.console:
        _dynStruct.print_to_console(_dynStruct.l_struct)
        
    if args.web_view:
        _dynStruct.start_webui(args.bind_addr.split(":")[0],
                              args.bind_addr.split(":")[1] if ":" in args.bind_addr else 24242)
    
    
if __name__ == '__main__':
    main()
