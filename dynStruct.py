#!/usr/bin/python3

import argparse
import pickle
import json
import _dynStruct

def get_args():
    parser = argparse.ArgumentParser(description='Dynstruct analize tool')
    parser.add_argument('-d', type=str, dest='dynamo_file',
                        help='output file from dynStruct dynamoRio client')
    parser.add_argument('-p', type=str, dest='previous_file',
                        help='file to load serialized data')
    parser.add_argument('-o', type=str, dest='out_pickle',
                        help='file to store serialized data.')
    parser.add_argument('-e', type=str, default=None, dest='out_file',
                        metavar='<file_name>',
                        help='export structures in C style on <file_name>')
    parser.add_argument('-c', action='store_true', dest='console',
                        help='print structures in C style on console')
    parser.add_argument('-w', action='store_false', dest='web_view',
                        help='start the web view')
    parser.add_argument('-l', dest='bind_addr', default='127.0.0.1', type=str,
                        help='bind addr for the web view default 127.0.0.1')

    return parser.parse_args()



def load_json(json_data, l_block, l_access_w, l_access_r):

    try:
        for block in filter(None, json_data):
            l_block.append(_dynStruct.Block(block, l_access_w, l_access_r))
    except KeyError as e:
        print("Json not from dynamoRIO client, missing : %s" % str(e))
        return False
    return True

def load_pickle(pickle_file):
    unpickler = pickle.Unpickler

def main():
    args = get_args()

    l_struct = []
    l_block = []
    l_access_w = []
    l_access_r = []
    
    if args.dynamo_file:
        f = open(args.dynamo_file, "r")
        json_data = json.load(f)
        f.close()
        load_json(json_data, l_block, l_access_w, l_access_r)
        _dynStruct.Struct.recover_all_struct(l_block, l_struct);
        _dynStruct.Struct.clean_all_struct(l_struct)
    elif args.previous_file:
        with open(args.previous_file, "rb") as f:
            data = pickle.load(f)
        l_struct = data["structs"]
        l_block = data["blocks"]
        l_access_w = data["w_access"]
        l_access_r = data["r_access"]
            
    if args.out_pickle:
        data = {"structs" : l_struct, "blocks" : l_block}
        data["w_access"] = l_access_w
        data["r_access"] = l_access_r
        with open(args.out_pickle, "wb") as f:
            pickle.dump(data, f, pickle.HIGHEST_PROTOCOL)

    if args.out_file:
        _dynStruct.print_to_file(args.out_file, l_struct)

    if args.console:
        _dynStruct.print_to_console(l_struct)
        
#    if args.web_view:
        # start web_view
    
    
if __name__ == '__main__':
    main()
