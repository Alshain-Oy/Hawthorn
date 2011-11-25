#!/usr/bin/env python

import sys, os, json, time, urllib
import pprint

def send_data( data ):
	p = urllib.quote_plus( json.dumps( {'query': data} ) )
	
	handle = urllib.urlopen( 'https://127.0.0.1:9150', p )
	data = handle.read()
	handle.close()
	return json.loads( data )
	
	

txt = ""

with open( sys.argv[1], 'r' ) as handle:
	txt = handle.read()

rlines = txt.strip().splitlines()

lines = []
for line in rlines:
	line = line.strip()
	if '#' in line:
		line = line[ :line.index( "#" ) ].strip()
	if len( line ) > 0:
		lines.append( line )
	


#pprint.pprint( send_data( lines ) )

print json.dumps( send_data( lines ), sort_keys = True, indent = 4 )

