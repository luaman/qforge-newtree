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

class cvar:
	def __init__(self, name, default, flags, desc, source):
		self.name = name
		self.default = default
		self.flags = string.split (flags, '|')
		self.desc = desc
		self.source = source
	def __repr__(self):
		return 'cvar(('+`self.name`+','+\
						`self.default`+','+\
						`string.join(self.flags,'|')`+','+\
						`self.desc`+','+\
						`self.source`+')'
	def __str__(self):
		return self.name+'\n\t'+self.default+'\n\t'+`self.flags`+'\n\t'+self.desc+'\n\t'+self.source[0]+':'+`self.source[1]`

cvars = []

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
			g = m.groups()
			if g[2]:
				name = g[2]
				default = clean(g[4])
				flags = g[11]
				desc = clean(g[12])
				cvars.append (cvar (name, default, flags, desc, (fname, i+1)))
			i=i+j
		else:
			i=i+1

for f in sys.argv[1:]:
	get_cvar_defs(f)
for c in cvars:
	print c
