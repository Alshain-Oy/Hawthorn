#!/usr/bin/env python

import libHawthorn as Haw

import threading

commands = [ 'select-graph', 'create-graph', 'create-node', 'connect-nodes', 
'disconnect-nodes', 'get-node', 'get-results', 'add-property', 'get-property', 
'is-connected', 'find-shortest-path', 'find-longest-path', 'find-nearest-nodes', 
'compute-path-length', 'get-children', 'get-parents', 'filter-results', 'union', 
'intersection', 'complement', 'remove-node', 'remove-graph', 'remove-results', 'get-connected', 'colour-graph' ]

write_operations = [ 'create-graph', 'create-node', 'connect-nodes', 'disconnect-nodes', 'remove-node', 'remove-graph' ]

import time, random, os, hashlib




def escape( s ):
	return s.replace( '\n', '\\n' ).replace( '\r', '\\r' ).replace( '"', '\\"' )

def split_line( line ):
	out = []
	state = False
	pos = 0
	while pos < len( line ):
		#print pos, len(line), line, state
		if line[pos] == ' ':
			if not state:
				#print repr(line[:pos])
				out.append( line[:pos] )
				line = line[pos+1:]
				pos = 0
				continue
			else:
				pos += 1
		elif line[pos] == '\\':
			pos += 2
			continue
		elif line[pos] == '"':
			if state:
				state = False
				pos += 1
				continue
			else:
				state = True
				pos += 1
				continue
			
		else:		
			pos += 1
		#time.sleep( 0.3 )
	if len( line ) > 0:
		out.append( line )
	
	fout = []
	for o in out:
		if len( o.strip() ) > 0:
			if o.startswith( '"' ):
				o = o[1:-1]
			#print o, str( o ), repr(o), re.escape( o )
			fout.append( escape( o.strip() ) )
	
	return fout
	
def parse_line( line ):
	global commands
	#parts = line.split()
	parts = split_line( line )
	cmd = parts[0]
	if cmd not in commands:
		raise Exception( "Invalid command %s."%repr( cmd ) )
	return parts

def pnode( tag ):
	if tag.startswith( 'id:' ):
		return Haw.Hawthorn.encode_value( int( tag[3:] ) )
	else:
		return Haw.Hawthorn.encode_value( tag )

class HawthornQuery( object ):
	def __init__( self, client, write_lock, dummy ):
		self.write_lock = write_lock
		self.client = client
	
	def start( self ):
		self.client.connect()
	
	def stop( self ):
		self.client.disconnect()
	
	def query( self, lines ):
		batch = Haw.HawthornBatch()
		for line in lines:
			parts = parse_line( line )
			cmd = str( parts[0] )
			params = [ str(x) for x in parts[1:] ]
			
			E = Haw.Hawthorn.encode_value
			
			if cmd == 'select-graph' or cmd == 'create-graph':
				batch.set_graph( params[0] )
			
			elif cmd == 'create-node':
				s = Haw.HawthornStatement( Haw.Hawthorn.CREATE_NODE, "", [ E( params[0] ) ] )
				batch.add_statement( s )

			elif cmd == 'connect-nodes':
				s = Haw.HawthornStatement( Haw.Hawthorn.CONNECT_NODES, "", [ pnode( params[0] ), pnode( params[1] ), E( float( params[2]) ) ] )
				batch.add_statement( s )
				
			elif cmd == 'disconnect-nodes':
				s = Haw.HawthornStatement( Haw.Hawthorn.DISCONNECT_NODES, "", [ pnode( params[0] ), pnode( params[1] ) ] ) 
				batch.add_statement( s )
			
			elif cmd == 'get-node':
				s = Haw.HawthornStatement( Haw.Hawthorn.GET_NODE, params[0], [ pnode( params[1] ) ] )
				batch.add_statement( s )
			
			elif cmd == 'find-shortest-path':
				s = Haw.HawthornStatement( Haw.Hawthorn.FIND_SHORTEST_PATH, params[0], [ pnode( params[1] ), pnode( params[2] ) ] )
				batch.add_statement( s )
			
			elif cmd == 'find-longest-path':
				s = Haw.HawthornStatement( Haw.Hawthorn.FIND_LONGEST_PATH, params[0], [ pnode( params[1] ), pnode( params[2] ) ] )
				batch.add_statement( s )
		
			elif cmd == 'find-nearest-nodes':
				s = Haw.HawthornStatement( Haw.Hawthorn.FIND_NEAREST_NODES, params[0], [ pnode( params[1] ), E( float( params[2] ) ) ] ) 
				batch.add_statement( s )
		
			elif cmd == 'get-children':
				s = Haw.HawthornStatement( Haw.Hawthorn.GET_CHILDREN, params[0], [ E(params[1]) ] )
				batch.add_statement( s )


			elif cmd == 'get-parents':
				s = Haw.HawthornStatement( Haw.Hawthorn.GET_PARENTS, params[0], [ E(params[1]) ] )
				batch.add_statement( s )

			elif cmd == 'filter-results':
				s = Haw.HawthornStatement( Haw.Hawthorn.FILTER, params[0], [ E(params[1]), E(params[2]), E(params[3]) ] )
				batch.add_statement( s )
		
			elif cmd == 'union':
				s = Haw.HawthornStatement( Haw.Hawthorn.UNION, params[0], [ E(params[1]), E(params[2]) ] )
				batch.add_statement( s )
			
			elif cmd == 'intersection':
				s = Haw.HawthornStatement( Haw.Hawthorn.INTERSECT, params[0], [ E(params[1]), E(params[2]) ] )
				batch.add_statement( s )
			
			elif cmd == 'complement':
				s = Haw.HawthornStatement( Haw.Hawthorn.COMPLEMENT, params[0], [ E(params[1]), E(params[2]) ] )
				batch.add_statement( s )
			
			elif cmd == 'remove-node':
				s = Haw.HawthornStatement( Haw.Hawthorn.REMOVE_NODE, params[0], [ E(params[1]), E(params[2]) ] )
				batch.add_statement( s )
			
			elif cmd == 'get-connected':
				s = Haw.HawthornStatement( Haw.Hawthorn.GET_CONNECTED, params[0], [ pnode( params[1] ) ] )
				batch.add_statement( s )
			
			elif cmd == 'add-property':
				s = Haw.HawthornStatement( Haw.Hawthorn.ADD_PROPERTY, "", [ pnode( params[0] ), E(params[1]), E(params[2]) ] )
				batch.add_statement( s )
			
			elif cmd == 'get-property':
				s = Haw.HawthornStatement( Haw.Hawthorn.GET_PROPERTY, params[0], [ pnode( params[0] ), E(params[1]) ] )
				batch.add_statement( s )
			
			elif cmd == 'remove-property':
				s = Haw.HawthornStatement( Haw.Hawthorn.REMOVE_PROPERTY, "", [ pnode( params[0] ), E(params[1]) ] )
				batch.add_statement( s )
			
			elif cmd == 'colour-graph':
				s = Haw.HawthornStatement( Haw.Hawthorn.COLOUR_GRAPH, params[0], [] )
				batch.add_statement( s )
			
			elif cmd == 'get-results':
				s = Haw.HawthornStatement( Haw.Hawthorn.GET_RESULTS, params[0], [] )
				batch.add_statement( s )
			
			elif cmd == 'compute-path-length':
				s = Haw.HawthornStatement( Haw.Hawthorn.COMPUTE_PATH_LENGTH, params[0], [ E(params[1]) ] )
				batch.add_statement( s )
			
			elif cmd == 'remove-results':
				s = Haw.HawthornStatement( Haw.Hawthorn.REMOVE_RESULTS, params[0], [] )
				batch.add_statement( s )
			
			
		response = self.client.execute_batch( batch )
		#print "DEBUG: response:", response
		#print "DEBUG: response.results:", response.results
		
		return response
