-- using 1743080388 as a seed to the RNG


select
	sum(l_extendedprice) / 7.0 as avg_yearly
from
	lineitem,
	part,
	(SELECT l_partkey AS agg_partkey, 0.2 * avg(l_quantity) AS avg_quantity FROM lineitem GROUP BY l_partkey) part_agg
where
	p_partkey = l_partkey
	and agg_partkey = l_partkey
	and p_brand = 'Brand#32'
	and p_container = 'LG PACK'
	and l_quantity < avg_quantity
LIMIT 1;
