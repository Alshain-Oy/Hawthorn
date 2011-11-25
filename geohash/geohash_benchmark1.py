#!/usr/bin/env python
import math

#
# |---------|
# | 00 | 01 |
# |----|----|
# | 10 | 11 |
# |____|____|

def compute_hash( N, x, y ):
	out = 0
	
	cx = 0.5
	cy = 0.5
	
	d = 0.25
	
	x = x - math.floor(x)
	y = y - math.floor(x)
	
	
	for i in range( N ):
		p = 0
		if x >= cx:
			p |= 1
			cx += d
			#print "X >=", cx, 
		else:
			cx -= d
			#print "X <", cx,
			
		if y <= cy:
			p |= 2
			cy -= d
			#print "Y <=", cy,
		else:
			cy += d
			#print "Y >", cy,
		
		#print bin(p)
		#print p
		d /= 2.0
		out |= p
		out <<= 2
	#out >>= 2
	return out

def split_hash( H, M = 16 ):
	parts = []
	for i in range( M ):
		p = 2*(M-i)
		z = 3 << p
		parts.append( (H & z) >> p )
	return parts

def mask_hash( H, N, M = 32 ):
	mask = 0
	mask2 = 0
	for i in range(M):
		if i < N:
			mask |= 0x3
		else:
			mask2 |= 0x3
		mask <<= 2
		mask2 <<= 2
	#print bin(H), bin(mask), bin(mask | mask2)
	return H & mask, (H & mask) | mask2
	
def compute_pos( H ):
	parts = split_hash( H )
	#print parts, len(parts)
	d = 0.25
	cx = 0.5
	cy = 0.5
	
	for i in range( len( parts ) ):
		p = parts[i]
		x = p & 1
		y = p & 2
		#print p, (x,y)
		if x>0:
			cx += d
		else:
			cx -= d
		
		if y>0:
			cy -= d
		else:
			cy += d
		d /= 2
	return (cx,cy)
		

def list_to_int( L ):
	L.reverse()
	out = 0
	for i in range( len( L ) ):
		out += (2**(2*i)) * L[i]
	return out 
	
def get_near_indices(M, N, H ):
	dz = 1.0/(2**(N))
	pos = compute_pos( H )
	out = []
	for p in [-1,0,1]:
		for q in [-1,0,1]:
			x = pos[0] + dz * p
			y = pos[1] + dz * q
			h = compute_hash( M, x, y )
			entry = mask_hash( h, N, M )
			if entry not in out:
				out.append( entry )
	return out
			
			
def merge_search( indices ):
	out = []
	flatten = []
	for i in indices:
		flatten.append( i[0] )
		flatten.append( i[1] )
	flatten.sort()
	pos = 0
	#for i in range( len(flatten) - 1 ):
	while pos < len( flatten ) - 1:
		if flatten[pos] + 1 == flatten[pos+1] and False:
			#print pos
			pos += 2
		else:
			out.append( flatten[pos] )
			pos += 1
	out.append( flatten[pos] )
	return [ (out[i], out[i+1]) for i in range( 0, len(out), 2 ) ] 
	
	

import random, bisect

def get_within_radius( points, p0, r ):
	dist = lambda a,b: ( (a[0]-b[0])**2 + (a[1]-b[1])**2) < r*r
	
	out = []
	for p in points:
		if dist( points[p], p0 ):
			out.append( p )
	return out
	

def get_in_area( hdb, indices, search_area ):
	out = []
	keys = []
	for A in search_area:
		a0 = bisect.bisect_left( indices, A[0] )
		a1 = bisect.bisect_left( indices, A[1] )
		keys.extend( indices[a0:a1] )
	
	for k in keys:
		out.append( hdb[k] )
	out.sort()
			
	
	return out
		
def select_keys( d, keys ):
	out = {}
	for k in keys:
		out[k] = d[k]
	return out


points = {}

M = 2e6


import time

print "Initializing..."
for i in range( int(M) ):
	x = random.random()
	y = random.random()
	points[i] = ( x, y )

indices = []


Nbits = 32

hdb = {}

test_point = (0.5,0.5)
#test_point = (random.random(),random.random())
H_test_point = compute_hash( Nbits, test_point[0], test_point[1] )


for p in points:
	H = compute_hash( Nbits, points[p][0], points[p][1] )
	hdb[H] = p
	bisect.insort_left( indices, H )
	
N = 15
r = 1.0/2**N
print "Starting benchmark..."
t0 = time.clock()
euc_idx = get_within_radius( points, test_point, r )
t1 = time.clock()

print "Linear:", t1-t0

t2 = time.clock()
idx = get_near_indices( Nbits, N, H_test_point )
search_area = merge_search( idx )
h_idx = get_in_area( hdb, indices, search_area )
final_idx = get_within_radius( select_keys( points, h_idx ), test_point, r )
t3 = time.clock()

print "Quadtree:", t3-t2	

print "Same results:", set(euc_idx) == set( final_idx )

print "Speed up factor: ", (t1-t0)/(t3-t2)
