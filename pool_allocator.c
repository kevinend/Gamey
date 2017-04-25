#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {

	uint8_t alignment;
	
	uint8_t *current;

	int block_size;
	int bytes_left;

} Pool;

Pool
pool_init( int requested_block_size, int requested_alignment ) 
{
	Pool pool;
	
	pool.alignment = requested_alignment;
	pool.current = NULL;
	pool.block_size = requested_block_size;
	pool.bytes_left = requested_block_size;

	return pool;
}

uint8_t *
pool_alloc( int requested_size, Pool *pool ) 
{
	if ( pool->current == NULL ) {
		pool->current = (uint8_t *)malloc( requested_size );
		if ( !pool->current ) {
			fprintf( stderr, "pool_alloc()...initial allocation failed\n" );
			return NULL;
		}
	}
	
	if ( pool->bytes_left < requested_size ) {
		fprintf( stderr, "pool_alloc()...out of memory exception\n" );
		return NULL;
	}

	int alignment_adjustment;
	alignment_adjustment = requested_size % pool->alignment;
	if ( alignment_adjustment != 0 ) {     
			if ( requested_size < pool->alignment ) {
				requested_size = pool->alignment;
			}
			else {
				requested_size += pool->alignment - (alignment_adjustment);  
			}
	}

	uint8_t *block_start;

	block_start = pool->current;
	pool->current += requested_size;
	pool->bytes_left -= requested_size;

	return block_start;	
}

// Pool is freed in one fell swoop
void
pool_free( Pool *pool ) 
{
	int total_allocated;

	total_allocated = pool->block_size - pool->bytes_left;
	printf( "Total Allocated == %d\n", total_allocated );
	printf( "Starting value == %d\n", *(pool->current - total_allocated) );
	free( pool->current - total_allocated );

	return;
}

