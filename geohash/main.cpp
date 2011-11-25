#include <iostream>
#include <map>
#include <set>

#include <time.h>
#include <sys/time.h>

//#include <windows.h>

#include "libhash.h"

double get_time(){
	struct timeval tv;
	gettimeofday( &tv, NULL );
	return tv.tv_sec + tv.tv_usec * 1e-6;
	//return time( NULL );
	//return GetTickCount() / 1000.0;
	}


void print_set( std::set<uint64_t> res ){
	for( std::set<uint64_t>::iterator it = res.begin() ; it != res.end() ; it++ ){
		std::cout << *it << " ";
		}
	std::cout << std::endl;
	}

int main( int argc, char **argv ){

	
	std::set< uint64_t > hashes, res_euc, res_hash;
	std::map< uint64_t, uint64_t > db;
	
	std::pair<double, double> test_point = std::make_pair( 0.5, 0.5 );
	uint64_t test_point_h = compute_hash( test_point.first, test_point.second );
	
	int N = 15;
	double radius = 1.0/(1<<N);
	
	std::vector< std::pair<double, double> > points;
	
	double t10 = get_time();
	
	for( size_t i = 0 ; i < 6e6 ; ++i ){
		double x, y;
		x = rand() * 1.0 / RAND_MAX;
		y = rand() * 1.0 / RAND_MAX;
		points.push_back( std::make_pair( x, y ) );
		
		uint64_t H = compute_hash( x, y );
		hashes.insert( H );
		db[ H ] = i;
		
		}
	
	double t11 = get_time();
	
	std::cout << "Initializing took "<< t11-t10 << std::endl;
	
	double t0, t1, t2, t3;
	
	t0 = get_time();
	for( size_t i = 0 ; i < points.size() ; ++i ){
		if( get_squared_distance( points[i], test_point ) < radius * radius ){
			res_euc.insert( i );
			}
		} 
	
	t1 = get_time();
	
	std::cout<< "It took " << t1 - t0 << " secs" << std::endl;
	//print_set( res_euc );
	
	t2 = get_time();
	size_t iter = 50000;
	for( size_t i = 0 ; i < iter ; ++i ){
		std::vector< std::pair< uint64_t, uint64_t > > areas = get_near_indices( N, test_point_h );
		std::set<uint64_t> results_inter = extract_points( hashes,  areas );
		res_hash = filter_points( results_inter, db, test_point, radius );
		}
	t3 = get_time();
	
	std::cout<< "It took " << (t3 - t2)/iter << " secs" << std::endl;
	//print_set( res_hash );
	std::cout << "Speed-up factor " << (t1-t0)/((t3 - t2)/iter) << std::endl;
	
	return 0;
	}
