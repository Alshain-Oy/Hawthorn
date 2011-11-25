#include <iostream>
#include <libgraph.hpp>

double get_time(){
	struct timeval tv;
	gettimeofday( &tv, NULL );
	return tv.tv_sec + tv.tv_usec * 1e-6;
	}


int main( int argc, char **argv ){
	
	NodeStorage store( 0xFFFF );
	Graph graph( &store );
	
	store.open_storage( "graph.db" );
	
	double t0, t1, t2, t3; 
	
	t0 = get_time();
	
	graph.restore_from_storage();
	
	t1 = get_time();
	
	std::cout << "It took " << t1-t0 << " secs to restore db from storage." << std::endl;
	
	std::vector< std::pair<double, std::pair<uint64_t,uint64_t> > > edges = graph.get_edges();
	
	t2 = get_time();
	std::cout << "Fetching and reading " << edges.size()/1000 << " kconns took " << t2-t1 << " secs." << std::endl;
	
	edges = graph.get_edges();
	
	t3 = get_time();
	
	std::cout << "Reading " << edges.size()/1000 << " kconns took " << t3-t2 << " secs." << std::endl;
	
	
	return 0;
	}

