# retrieve newick data from php and use
# the ete package to present phylogenetic tree.

#!/usr/bin/python2.6
import cgi
import cgitb; cgitb.enable()
import sys
import urllib
from ete2 import PhyloTree, Tree

tree = str(urllib.unquote_plus(sys.argv[1]))
tree = tree.lstrip('.')
tree = Tree(tree)
print "<pre>"+tree+"</pre>"
