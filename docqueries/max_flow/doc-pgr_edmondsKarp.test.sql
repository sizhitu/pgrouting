
/* -- q1 */
SELECT * FROM pgr_edmondsKarp(
    'SELECT id,
            source,
            target,
            capacity,
            reverse_capacity
    FROM edge_table'
    , 6, 11
);

/* -- q2 */
SELECT * FROM pgr_edmondsKarp(
    'SELECT id,
            source,
            target,
            capacity,
            reverse_capacity
    FROM edge_table'
   , 6, ARRAY[1, 3, 11]
);

/* -- q3 */
SELECT * FROM pgr_edmondsKarp(
    'SELECT id,
            source,
            target,
            capacity,
            reverse_capacity
    FROM edge_table'
   , ARRAY[6, 8, 12], 11
);

/* -- q4 */
SELECT * FROM pgr_edmondsKarp(
    'SELECT id,
            source,
            target,
            capacity,
            reverse_capacity
    FROM edge_table'
   , ARRAY[6, 8, 12], ARRAY[1, 3, 11]
);

/* -- q5 */
SELECT * FROM pgr_edmondsKarp(
    'SELECT id,
            source,
            target,
            capacity,
            reverse_capacity
    FROM edge_table',
    'SELECT * FROM ( VALUES (6, 1), (8, 3), (12, 11), (8, 1) ) AS t(source, target)'
);

/* -- q6 */
