BEGIN;


SELECT plan(1024);

UPDATE edge_table SET cost = sign(cost) + 0.001 * id * id, reverse_cost = sign(reverse_cost) + 0.001 * id * id;

SELECT compare_trspViaV_dijstraVia(16, true);

-- Finish the tests and clean up.
SELECT * FROM finish();
ROLLBACK;

