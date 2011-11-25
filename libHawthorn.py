#!/usr/bin/env python

import socket, struct

class Hawthorn( object ):
	CREATE_NODE = 1
	CONNECT_NODES = 2
	DISCONNECT_NODES = 3
	ADD_PROPERTY = 4
	GET_PROPERTY = 5
	REMOVE_PROPERTY = 6
	IS_CONNECTED = 7
	GET_NODE = 8
	FIND_SHORTEST_PATH = 9
	FIND_LONGEST_PATH = 10
	FIND_NEAREST_NODES = 11
	GET_CHILDREN = 12
	GET_PARENTS = 13
	FILTER = 14
	UNION = 15
	INTERSECT = 16
	COMPLEMENT = 17
	REMOVE_NODE = 18
	GET_RESULTS = 19
	GET_CONNECTED = 20
	REMOVE_RESULTS = 21
	COLOUR_GRAPH = 22
	COMPUTE_PATH_LENGTH = 23
	
	@staticmethod
	def decode_value( value ):
		if value.startswith( 's:' ):
			return value[2:]
		elif value.startswith( 'i:'):
			return int( value[2:] )
		elif value.startswith( 'u:'):
			return struct.unpack( "<Q", value[2:] )[0]
		elif value.startswith( 'f:' ):
			return float( value[2:] )
		elif value.startswith( 'n:' ):
			return HawthornNode( value[2:] )
		else:
			print "VALUE:", value
	
	@staticmethod
	def encode_value( value ):
		if isinstance( value, str ):
			return "s:" + value
		elif isinstance( value, int ) or isinstance( value, long ):
			return "i:" + str(value)
		elif isinstance( value, float ):
			return "f:" + str(value)

def pstr( s ):
	out = struct.pack( "<Q", len( s ) )
	out += s
	return out

def pint( i ):
	return struct.pack( "<Q", i )

class HawthornStatement( object ):
	def __init__( self, cmd, results, params ):
		self.cmd = cmd
		self.results = results
		self.params = params
	
	def serialize( self ):
		out = ""
		out += pint( self.cmd )
		out += pstr( self.results )
		out += pint( len( self.params ) )
		for p in self.params:
			out += pstr( p )
		
		#out = pint( len( out ) + struct.calcsize( "Q" ) ) + out
		return out
		
class HawthornBatch( object ):
	def __init__( self ):
		self.graph = None
		self.statements = []
	
	def set_graph( self, g ):
		self.graph = g
	
	def add_statement( self, s ):
		self.statements.append( s )
	
	def serialize( self ):
		out = ""
		out += pstr( self.graph )
		out += pint( len( self.statements ) )
		for s in self.statements:
			out += pstr( s.serialize() )
			
		out = pint( len( out ) + struct.calcsize( "Q" ) ) + out
		return out

class HawthornResponse( object ):
	def __init__( self, data ):
		self.status = 0
		self.results = []
		(total_len, status, result_len ) = struct.unpack( "<QQQ", data[:struct.calcsize( "<QQQ" ) ] )
		pos = struct.calcsize( "<QQQ" )
		for i in range( result_len ):
			L = struct.unpack( "<Q", data[pos:pos+struct.calcsize("<Q")])[0]
			pos += struct.calcsize( "<Q" )
			self.results.append( Hawthorn.decode_value( data[ pos : pos + L ] ) )
			pos += L
			

class HawthornNode( object ):
	def __init__( self, data = None ):
		self.id = 0
		self.name = ""
		self.conns = {}
		self.bconns = []
		self.properties = {}
		if data:
			self.load( data )
	
	def load( self, data ):
		#print "HawthornNode.load: len(data) =", len( data )
		self.id = struct.unpack( "<Q", data[:struct.calcsize( "Q" ) ] )[0]
		#print "self.id:", self.id
		pos = struct.calcsize( "Q" )
		#print "pos:", pos
		name_len = struct.unpack( "<Q", data[ pos: pos + struct.calcsize( "Q" ) ] )[0]
		#print "name_len:", name_len
		pos += struct.calcsize( "Q" )
		self.name = data[ pos:pos+name_len]
		pos += name_len
		#print "pos:", pos
		
		conns_len = struct.unpack( "<Q", data[ pos: pos + struct.calcsize( "Q" ) ] )[0]
		pos += struct.calcsize( "Q" )
		for i in range( conns_len ):
			conn_id = struct.unpack( "<Q", data[ pos: pos + struct.calcsize( "Q" ) ] )[0]
			pos += struct.calcsize( "Q" )
			w = struct.unpack( "<d",  data[ pos: pos + struct.calcsize( "d" ) ] )[0]
			pos += struct.calcsize( "d" )
			self.conns[ conn_id ] = w
		
		conns_len = struct.unpack( "<Q", data[ pos: pos + struct.calcsize( "Q" ) ] )[0]
		pos += struct.calcsize( "Q" )
		for i in range( conns_len ):
			conn_id = struct.unpack( "<Q", data[ pos: pos + struct.calcsize( "Q" ) ] )[0]
			pos += struct.calcsize( "Q" )
			self.bconns.append( conn_id )
		
		conns_len = struct.unpack( "<Q", data[ pos: pos + struct.calcsize( "Q" ) ] )[0]
		pos += struct.calcsize( "Q" )
		for i in range( conns_len ):
			plen = struct.unpack( "<Q", data[ pos: pos + struct.calcsize( "Q" ) ] )[0]
			pos += struct.calcsize( "Q" )
			key = data[ pos:pos+plen]
			pos += plen
			plen = struct.unpack( "<Q", data[ pos: pos + struct.calcsize( "Q" ) ] )[0]
			pos += struct.calcsize( "Q" )
			value = data[ pos:pos+plen]
			pos += plen
			self.properties[ key ] = value
		
		
	def to_dict( self ):
		out = {}
		out['id'] = self.id
		out['name'] = self.name
		out['connections'] = self.conns
		out['back_connections'] = self.bconns
		out['properties'] = self.properties
		return out

class HawthornClient( object ):
	def __init__( self, address = '/tmp/hawthorn.socket' ):
		self.address = address
		self.sock = socket.socket( socket.AF_UNIX, socket.SOCK_STREAM )
	
	def connect( self ):
		self.sock.connect( self.address )

	def disconnect( self ):
		self.sock.close()
	
	def execute_batch( self, batch ):
		data = batch.serialize()
		L = 0
		while L < len( data ):
			n = self.sock.send( data[L:] )
			if n > 0:
				L += n
		
		response = ""
		done = False
		L = None
		while not L:
			response += self.sock.recv( 1 )
			if not L:
				if len( response ) > struct.calcsize( "Q" ):
					L = struct.unpack( "<Q", response[:struct.calcsize("Q")] )[0]
			
		while not done:
			response += self.sock.recv( L - len( response ) )
			if len( response ) >= L:
				done = True

		return HawthornResponse( response )



#tests
#
#
#client = HawthornClient()
#
#batch = HawthornBatch()
#batch.set_graph( "test1" )
#batch.add_statement( HawthornStatement( Hawthorn.CREATE_NODE, "", ["s:nodeA" ] ) )
#batch.add_statement( HawthornStatement( Hawthorn.CREATE_NODE, "", ["s:nodeB" ] ) )
#batch.add_statement( HawthornStatement( Hawthorn.CREATE_NODE, "", ["s:nodeC" ] ) )
#batch.add_statement( HawthornStatement( Hawthorn.CREATE_NODE, "", ["s:nodeD" ] ) )
#batch.add_statement( HawthornStatement( Hawthorn.CREATE_NODE, "", ["s:nodeE" ] ) )
#batch.add_statement( HawthornStatement( Hawthorn.CREATE_NODE, "", ["s:nodeF" ] ) )
#batch.add_statement( HawthornStatement( Hawthorn.CREATE_NODE, "", ["s:nodeG" ] ) )
#
#batch.add_statement( HawthornStatement( Hawthorn.CONNECT_NODES, "", ["s:nodeA", "s:nodeB", "f:1.0" ] ) )
#batch.add_statement( HawthornStatement( Hawthorn.CONNECT_NODES, "", ["s:nodeB", "s:nodeA", "f:1.0" ] ) )
#
#batch.add_statement( HawthornStatement( Hawthorn.CONNECT_NODES, "", ["s:nodeA", "s:nodeC", "f:1.0" ] ) )
#batch.add_statement( HawthornStatement( Hawthorn.CONNECT_NODES, "", ["s:nodeC", "s:nodeA", "f:1.0" ] ) )
#
#batch.add_statement( HawthornStatement( Hawthorn.CONNECT_NODES, "", ["s:nodeC", "s:nodeB", "f:1.0" ] ) )
#batch.add_statement( HawthornStatement( Hawthorn.CONNECT_NODES, "", ["s:nodeB", "s:nodeC", "f:1.0" ] ) )
#
#batch.add_statement( HawthornStatement( Hawthorn.CONNECT_NODES, "", ["s:nodeD", "s:nodeB", "f:1.0" ] ) )
#batch.add_statement( HawthornStatement( Hawthorn.CONNECT_NODES, "", ["s:nodeB", "s:nodeD", "f:1.0" ] ) )
#
#batch.add_statement( HawthornStatement( Hawthorn.CONNECT_NODES, "", ["s:nodeC", "s:nodeE", "f:1.0" ] ) )
#batch.add_statement( HawthornStatement( Hawthorn.CONNECT_NODES, "", ["s:nodeE", "s:nodeC", "f:1.0" ] ) )

#batch.add_statement( HawthornStatement( Hawthorn.CONNECT_NODES, "", ["s:nodeD", "s:nodeE", "f:1.0" ] ) )
#batch.add_statement( HawthornStatement( Hawthorn.CONNECT_NODES, "", ["s:nodeE", "s:nodeD", "f:1.0" ] ) )

#batch.add_statement( HawthornStatement( Hawthorn.CONNECT_NODES, "", ["s:nodeC", "s:nodeF", "f:1.0" ] ) )
#batch.add_statement( HawthornStatement( Hawthorn.CONNECT_NODES, "", ["s:nodeF", "s:nodeC", "f:1.0" ] ) )

#batch.add_statement( HawthornStatement( Hawthorn.CONNECT_NODES, "", ["s:nodeE", "s:nodeG", "f:1.0" ] ) )
#batch.add_statement( HawthornStatement( Hawthorn.CONNECT_NODES, "", ["s:nodeG", "s:nodeE", "f:1.0" ] ) )

#batch.add_statement( HawthornStatement( Hawthorn.CONNECT_NODES, "", ["s:nodeF", "s:nodeG", "f:1.0" ] ) )
#batch.add_statement( HawthornStatement( Hawthorn.CONNECT_NODES, "", ["s:nodeG", "s:nodeF", "f:1.0" ] ) )

#batch.add_statement( HawthornStatement( Hawthorn.GET_NODE, "r:node", ["s:nodeA"] ) )
#batch.add_statement( HawthornStatement( Hawthorn.GET_RESULTS, "r:node", [] ) )
#batch.add_statement( HawthornStatement( Hawthorn.REMOVE_RESULTS, "r:node", [] ) )

#batch.add_statement( HawthornStatement( Hawthorn.COLOUR_GRAPH , "r:colour", [] ) )
#batch.add_statement( HawthornStatement( Hawthorn.GET_RESULTS, "r:colour", [] ) )
#batch.add_statement( HawthornStatement( Hawthorn.REMOVE_RESULTS, "r:colour", [] ) )


#client.connect()

#response = client.execute_batch( batch )
#
#print "response.status =", response.status
#
#for r in response.results:
#	print r

#client.disconnect()
