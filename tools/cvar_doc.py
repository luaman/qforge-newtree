#!/usr/bin/env python

import sys
import re
import string

cvar_def_re = re.compile(r'Cvar_Get\s*\(\s*(("' +
							 r'([^"]+)' +
						 r'")|([^,]+))\s*,\s*(("' +
							 r'((\\.|[^"\\])*(\n(\\.|[^"\\])*)*)' +
						 r'")|([^,]+))\s*,\s*' +
						 r'([^,]+)' +
						 r'\s*,\s*(("' +
						 r'((\\.|[^"\\])*(\n(\\.|[^"\\])*)*)' +
						 r'"\s*)+)\)\s*;')
cvar_get_re = re.compile(r'=\s*Cvar_Get')

def clean(str):
	str=string.strip(str)
	if str[0]=='"':
		str=str[1:]
	if str[-1]=='"':
		str=str[:-1]
	str = re.sub(r'\s*"\s*\n\s*"\s*', ' ', str)
	return str

def get_cvar_defs(fname):
	f=open(fname,'rt').readlines()
	i=0
	while (i<len(f)):
		if cvar_get_re.search(f[i]):
			j=1
			l = string.join(f[i:i+j],'')
			m = cvar_def_re.search(l)
			while not m and i+j<len(f):
				j=j+1
				l = string.join(f[i:i+j],'')
				m = cvar_def_re.search(l)
			i=i+j
			g = m.groups()
			if g[2]:
				name = g[2]
				default = clean(g[4])
				flags = g[11]
				desc = clean(g[12])
				print name
				print '\t'+default
				print '\t'+flags
				print '\t'+desc
		else:
			i=i+1

for f in sys.argv[1:]:
	get_cvar_defs(f)
