import xml.etree.ElementTree as ET
from xml.etree.ElementTree import Element
from typing_extensions import Self
import argparse

NODES_DATA_FILE = "btf_nodes_generated.c"
ACTION_FUNCTION_DATA_FILE_NO_EXT = "btf_action_functions_generated"

BTF_NODES_DATA_FILE_INCLUDE = \
f"""#include "btreefy/btreefy_policies.h"
#include "btreefy/btreefy_always_actions.h"
#include "{ACTION_FUNCTION_DATA_FILE_NO_EXT}.h"

"""

BTF_ACTION_FN_FILE_INCLUDE = \
"""#include "btreefy/btreefy_objs.h"

"""

BTF_NODE_STRUCT_CTYPE = "struct btf_node"
BTF_NODES_CARRAY_NAME = "nodes"
BTF_ACTION_FUNCTION_NAME_CTYPE = "btf_node_status_t {name}(btf_tree_st *tree, void *data, size_t datalen)"
BT_NODES_CARRAY_DECL_START = f"{BTF_NODE_STRUCT_CTYPE} {BTF_NODES_CARRAY_NAME}[] = {{"
BT_NODES_CARRAY_NODE_DECL_TEMPLATE = \
"""
    [{index}] = {{.status   = BTF_UNDEF_STATUS,
            .parent   = {parent},
            .child    =  {child},
            .sibling = {sibling},
            .action   = {action},
            .control  = {control},
            .name     = {name}}},
"""
BT_NODES_CARRAY_DECL_END = "};"
BTF_CTYPE_POLICY_FUNCTIONS_MAP = \
    {"Sequence": "btf_sequence_policy_fn",
     "Fallback": "btf_fallback_policy_fn",
     "ForceSuccess": "btf_success_policy_fn",
     "ForceFailure": "btf_fail_policy_fn"}

BTF_CTYPE_ALWAYS_ACTION_FUNCTIONS_MAP = \
    {"AlwaysSuccess": "btf_always_success_action",
     "AlwaysFailure": "btf_always_failure_action"}

class BTFNode:

    num_instances = 0
    nodes_d : dict [Element, Self] = {}

    def __init__(self, elem: Element, parent: Self = None, child: Self = None, sibling: Self = None):
        self.index: int = BTFNode.num_instances
        self._element: Element = elem
        self._parent: BTFNode = parent
        self._child: BTFNode = child
        self._sibling: BTFNode = sibling
        BTFNode.nodes_d.update([(elem, self)])
        BTFNode.num_instances += 1

    def __str__(self):
        info_str  = "Node info:\n"
        info_str += f"Index: {self.index}\n"
        elem_info = f"tag={self._element.tag}, attrib={self._element.attrib}" if self._element is not None else ""
        info_str += f"Element[{elem_info}]\n"
        info_str += f"Parent: {self._parent}\n"
        info_str += f"Child: {self._child}\n"
        info_str += f"Sibling: {self._sibling}\n"

        return info_str

    @classmethod
    def get_nodes_dict(cls) -> dict[Element, Self]:
        return cls.nodes_d
    
    @classmethod
    def index_of(cls, element) -> int:
        if element is None:
            return -1
        return cls.nodes_d.get(element).index

    @classmethod
    def get_node(cls, element) -> Self:
        return cls.nodes_d.get(element)

    @classmethod
    def _dfs_traversal(cls, node: Element, parent: Element, sibling: Element):
        bt_node = cls.get_node(node)

        bt_node.parent = cls.index_of(parent)
        bt_node.sibling = cls.index_of(sibling)
        bt_node.child = cls.index_of(node[0] if len(node) else None)

        children_l = node.findall("*")

        while children_l:
            c = children_l.pop(0) if len(children_l) else None
            p = node
            s = children_l[0] if len(children_l) else None
            cls._dfs_traversal(c, p, s)

    @classmethod
    def create_tree(cls, root: Element):
        for elem in root.iter():
            cls(elem)

        cls._dfs_traversal(root, None, None)

    @property
    def parent(self):
        return self._parent

    @parent.setter
    def parent(self, parent):
        self._parent = parent

    @property
    def sibling(self):
        return self._sibling

    @sibling.setter
    def sibling(self, sibling):
        self._sibling = sibling

    @property
    def child(self):
        return self._child

    @child.setter
    def child(self, child):
        self._child = child

    @property
    def element(self):
        return self._element

def main(args):
    # tree = ET.parse("../models/porta_automatica.xml")
    tree = ET.parse(args.model)

    root = tree.getroot()

    # bt = root.find("BehaviorTree[@ID='main_new']")[0]
    bt = root.find(f"BehaviorTree[@ID='{args.treename}']")[0]

    actions_fn_set = set()

    BTFNode.create_tree(bt)

    bt_nodes = BTFNode.get_nodes_dict()

    nodes_data_file = args.source_output_dir + '/' + NODES_DATA_FILE
    with open(nodes_data_file, mode='w') as nodes_f:
        nodes_f.write(BTF_NODES_DATA_FILE_INCLUDE)
        nodes_f.write(BT_NODES_CARRAY_DECL_START)
        for elem, node in bt_nodes.items():
            action_fn_str = "NULL"
            control_fn_str = "NULL"
            node_name = elem.attrib['name'] if 'name' in elem.attrib.keys() else elem.tag
            node_name = '"' + node_name + '"'
            if elem.tag in BTF_CTYPE_POLICY_FUNCTIONS_MAP.keys():
                control_fn_str = BTF_CTYPE_POLICY_FUNCTIONS_MAP[elem.tag]
            elif elem.tag in BTF_CTYPE_ALWAYS_ACTION_FUNCTIONS_MAP.keys():
                action_fn_str = BTF_CTYPE_ALWAYS_ACTION_FUNCTIONS_MAP[elem.tag]
            elif elem.tag in ("Script", "ScriptCondition"):
                action_fn_str = elem.attrib['code']
                actions_fn_set.add(action_fn_str)
            else:
                # TODO: Check if some action must be done
                pass
            s = BT_NODES_CARRAY_NODE_DECL_TEMPLATE.format(index=node.index, parent=node.parent, child=node.child, sibling=node.sibling, action=action_fn_str, control=control_fn_str, name=node_name)
            print(s)
            nodes_f.write(s)
        nodes_f.write(BT_NODES_CARRAY_DECL_END)

    action_fn_file = args.include_output_dir + '/' + ACTION_FUNCTION_DATA_FILE_NO_EXT+".h"
    with open(action_fn_file, mode='w') as actions_header_f:
        actions_header_f.write(BTF_ACTION_FN_FILE_INCLUDE)
        for action_fn in actions_fn_set:
            actions_header_f.write(BTF_ACTION_FUNCTION_NAME_CTYPE.format(name=action_fn) + ";")

            actions_header_f.write("\n\n")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog='BTreeFy Groot parser', description='Parses a Groot model and generate BTreeFy data models')
    
    parser.add_argument('-m', '--model', type=str, required=True, help="Groot model file with the Behavior Tree to be parsed")
    parser.add_argument('-tn', '--treename', type=str, required=True)
    parser.add_argument('--source-output-dir', type=str, default='.')
    parser.add_argument('--include-output-dir', type=str, default='.')

    args = parser.parse_args()

    main(args)