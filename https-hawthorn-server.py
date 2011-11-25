#!/usr/bin/env python

import socket, os

from SocketServer import ThreadingMixIn
from SocketServer import BaseServer
from BaseHTTPServer import HTTPServer, BaseHTTPRequestHandler
import threading
#from OpenSSL import SSL
import ssl

import urllib

import libHawthorn as Hawthorn
import libHawthornQuery as HawthornQuery

import json, sys

config = {}

#config['write_log'] = HawthornQuery.WriteLog( './logs/' )
config['write_lock'] = threading.Lock()

class RequestHandler( BaseHTTPRequestHandler ):
	def get_headers( self ):
		out = {}
		for key in self.headers:
			out[ key ] = self.headers[ key ]
		return out

	def do_GET( self ):
		self.send_response( 200 )
		self.end_headers()
		self.wfile.write( "Server running.\n" )
	
	def do_POST( self ):
		global config
		
		self.send_response( 200 )
		self.end_headers()
		
		client = Hawthorn.HawthornClient()
		query = HawthornQuery.HawthornQuery( client, config['write_lock'], None )
		headers = self.get_headers()
		
		data = urllib.unquote_plus( self.rfile.read( int( headers['content-length'] ) ) )
		lines = json.loads( data )['query']
		
		out = []
		
		query.start()
		
		#for line in lines:
		#	try:
		#		r = query.query( line ) 
		#	except:
		#		print sys.exc_info()
		#		out.append( {'status': str( sys.exc_info()[0] ) } )
		#		break
		#	else:
		#		if len( r ) < 1:
		#			out.append( {'status':'Ok.'} )
		#		else:
		#			out.append( r )
		
		rout = query.query( lines )
		
		
		query.stop()
		
		out = []
		
		for r in rout.results:
			if isinstance( r, Hawthorn.HawthornNode ):
				out.append( r.to_dict() )
			else:
				out.append( r )
		
		
		self.wfile.write( json.dumps( out ) )


class ThreadedServer( ThreadingMixIn, HTTPServer ):
	""" Threaded HTTP server """



if __name__ == '__main__':
	server_address = ('', 9150 )
	httpd = ThreadedServer( server_address, RequestHandler )
	httpd.socket = ssl.wrap_socket( httpd.socket, certfile="./server.pem", keyfile="./server.pem", server_side = True )
	httpd.serve_forever()
