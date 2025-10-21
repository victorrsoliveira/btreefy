import xml.etree.ElementTree as ET
from xml.etree.ElementTree import Element
from typing_extensions import Self

tree = ET.parse("../models/porta_automatica.xml")

root = tree.getroot()

bt = root.find("BehaviorTree[@ID='main_new']")

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

def node_info(node: Element, parent: Element, sibling: Element):
    info_str  = f"Node: {node.tag}\n"
    info_str += f"Attrib: {node.attrib}\n"
    info_str += f"Parent: {parent.tag if parent else None}\n"
    info_str += f"=" * 10
    info_str += "\n"
    return info_str

def dfs_traversal(node: Element, parent: Element, sibling: Element):
    bt_node = BTFNode.get_node(node)

    bt_node.parent = BTFNode.index_of(parent)
    bt_node.sibling = BTFNode.index_of(sibling)
    bt_node.child = BTFNode.index_of(node[0] if len(node) else None)

    print(bt_node)

    children_l = node.findall("*")

    while children_l:
        c = children_l.pop(0) if len(children_l) else None
        p = node
        s = children_l[0] if len(children_l) else None
        dfs_traversal(c, p, s)


for elem in bt.iter():
    BTFNode(elem)

bt_nodes = BTFNode.get_nodes_dict()

for node in bt_nodes.values():
    print(f"index: {node.index}")
    print(f"tag: {node.element.tag}")
    print(f"attrib: {node.element.attrib}")

dfs_traversal(bt, None, None)