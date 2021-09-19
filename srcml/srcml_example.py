import subprocess
import sys
from lxml import etree
from lxml.builder import ElementMaker

srcml_exe = 'srcml'

def to_xml(c_code):
    proc = subprocess.Popen([str(srcml_exe), '-lC'], stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE)
    streams = proc.communicate(input=c_code.encode('utf-8'))
    stdout, stderr = streams
    if proc.returncode != 0:
        raise Exception(stderr)
    return stdout

def to_c_code(xml):
    proc = subprocess.Popen([str(srcml_exe)], stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE)
    return proc.communicate(input=xml)[0].decode('utf-8')

namespaces = {'src': 'http://www.srcML.org/srcML/src'}
E = ElementMaker(namespace=namespaces["src"], nsmap=namespaces)

def xpath(node, query):
    """Return the result of an XPath query starting from node."""
    return node.xpath(query, namespaces=namespaces)

def get_tag(node):
    return etree.QName(node).localname

def join_str(*args):
    return ''.join(str(s) if s is not None else '' for s in args)

with open(sys.argv[1]) as f:
    c_code = f.read()

xml = to_xml(c_code)
root = etree.fromstring(xml)
for_stmt = xpath(root, '//src:for')[0]

# Components of the for statement
control = xpath(for_stmt, 'src:control')[0]
init = xpath(control, 'src:init')[0]
condition = xpath(control, 'src:condition')[0]
incr = xpath(control, 'src:incr')[0]
block = xpath(for_stmt, 'src:block')[0]
for_stmt_idx = for_stmt.getparent().index(for_stmt)

control_tail = control.tail
control_end = control.getchildren()[-1].tail
parent = for_stmt.getparent()
parent.remove(for_stmt)

block_content = block.getchildren()[0]
block_type = block.get('type')
last_child = block_content.getchildren()[-1]
incr.tail = join_str(';', for_stmt.tail)
last_child.tail = block_content.text
block_content.insert(len(block_content), incr)

init.tail = for_stmt.tail
parent.insert(for_stmt_idx, init)
for_stmt_idx += 1

condition.text = control.text
condition.tail = control.tail
cond_expr = xpath(condition, 'src:expr')[0]
cond_expr.tail = join_str(control_end)

args = [
    'while',
    'while',
    condition,
    block,
    for_stmt.tail,
]
new_while_stmt = E(*args)
parent.insert(for_stmt_idx, new_while_stmt)
xml = etree.tostring(root)
print(to_c_code(xml))
