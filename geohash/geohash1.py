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
	
	for i in range( N ):
		p = 0
		if x >= cx:
			p |= 1
			cx += d
		else:
			cx -= d
			
		if y <= cy:
			p |= 2
			cy -= d
		else:
			cy += d
		
		d /= 2.0
		out |= p
		out <<= 2
	return out

def split_hash( H ):
	parts = []
	while H > 0:
		parts.append( H & 0x3 )
		H >>= 2
	
	parts.reverse()
	return parts

	
def compute_pos( H ):
	parts = split_hash( H )
	d = 0.25
	cx = 0.5
	cy = 0.5
	
	for i in range( len( parts ) ):
		p = parts[i]
		x = p & 1
		y = p & 2
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
		

def compute_dist( N, H1, H2 ):
	Q = (H1 - H2) - (H1^H2)
	print bin(Q)
	c = 1
	out = 0
	for i in range( N ):
		d = 1 << (N-i)
		p = 2*(N-i)
		Z = 3 << p
		h1 = (H1 & Z) >> p
		h2 = (H2 & Z) >> p
		q = (Q & Z ) >> p
		#print q, bin(Q >> p)
		if not q:
			out += d * c
			c *= -1
			#Q = abs(~H1^H2)
			#print q, bin(Q >> p)
			Q = ~Q
	return out / (2.0**(N-2))
	#return abs(H1-H2) / (2.0**N)		
		
def list_to_int( L ):
	L.reverse()
	out = 0
	for i in range( len( L ) ):
		out += (2**(2*i)) * L[i]
	return out 
	
def get_near_indices(M, N, H ):
	dz = 1.0/(2**N)
	pos = compute_pos( H )
	out = []
	padding0 = [0] * (M-N)
	padding1 = [3] * (M-N)
	for p in [-1,0,1]:
		for q in [-1,0,1]:
			x = pos[0] + dz * p
			y = pos[1] + dz * q
			h = compute_hash( M, x, y )
			L = split_hash( h )
			tmp0 = L[:N]
			tmp0.extend( padding0 )
			tmp1 = L[:N]
			tmp1.extend( padding1 )
			entry = ( list_to_int(tmp0), list_to_int(tmp1) )
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
		if flatten[pos] + 1 == flatten[pos+1]:
			print pos
			pos += 2
		else:
			out.append( flatten[pos] )
			pos += 1
	out.append( flatten[pos] )
	return out
	
	
x0 = 0.49999
y0 = 0.1

x1 = 0.511
y1 = 0.11

N = 16

H1 = compute_hash( N, x0, y0 )
H2 = compute_hash( N, x1, y1 )
print H1, bin( H1 )
print H2, bin( H2 )
print ""
print "Distance", math.sqrt( (x0-x1)**2 + (y0-y1)**2 )
print ""
#print compute_dist( N, H1, H2 ), math.sqrt( (x0-x1)**2 + (y0-y1)**2 )
#print ""
print (x0,y0), compute_pos( H1 )
print ""
print "D(level=3):", 1.0/2**3
#diff = H1 ^ H2

#print diff, diff / float( 2**8 )
print ""

import pprint
idx = get_near_indices( 16, 3, H1 )
pprint.pprint(idx)
print ""
print merge_search( idx )
