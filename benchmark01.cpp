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
	
	std::vector<std::string> names;
	uint64_t N, C, id;
	char buf[32];
	double t0, t1, t2, t3, t4, t5;
	
	std::cerr << "Generating nodes...." << std::endl;
	
	N = 250000*4;
	C = 4*N;	
	t0 = get_time();
	for( size_t i = 0 ; i  < N ; ++i ){
		///std::cerr << i << std::endl;
		memset( buf, 0, 32 );
		sprintf( buf, "node%04i", i );
		id = graph.create_node( std::string( buf ) );
		graph.get_node( id )->add_property( "avain", "arvo12345678901234567890" );
		}
	t1 = get_time();
	
	std::cout << "Creating " << N/1000 << " knodes took " << t1-t0 << " secs." << std::endl;
	
	for( size_t i = 0 ; i  < C ; ++i ){
		uint64_t a, b;
		a = rand() % N;
		b = rand() % N;
		
		graph.connect_nodes( a, b, 1.0 );
		
		}
	t2 = get_time();
	
	std::cout << "Creating " << C / 1000 << " kconns took " << t2-t1 << " secs." << std::endl;
	
	for( size_t i = 0 ; i  < N ; ++i ){
		graph.storage()->suppress_node( i );
		}
	
	t3 = get_time();
	
	std::cout << "Suppressing " << N / 1000 << " knodes to storage took " << t3-t2 << " secs." << std::endl;
	
	std::vector< std::pair<double, std::pair<uint64_t,uint64_t> > > edges = graph.get_edges();
	
	t4 = get_time();
	std::cout << "Fetching and reading " << edges.size()/1000 << " kconns took " << t4-t3 << " secs." << std::endl;
	
	edges = graph.get_edges();
	
	t5 = get_time();
	
	std::cout << "Reading " << edges.size()/1000 << " kconns took " << t5-t4 << " secs." << std::endl;
	
	
	return 0;
	}

