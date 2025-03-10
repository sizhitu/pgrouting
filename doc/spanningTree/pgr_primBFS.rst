..
   ****************************************************************************
    pgRouting Manual
    Copyright(c) pgRouting Contributors

    This documentation is licensed under a Creative Commons Attribution-Share
    Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
   ****************************************************************************

|

* **Supported versions:**
  `Latest <https://docs.pgrouting.org/latest/en/pgr_primBFS.html>`__
  (`3.3 <https://docs.pgrouting.org/3.3/en/pgr_primBFS.html>`__)
  `3.2 <https://docs.pgrouting.org/3.2/en/pgr_primBFS.html>`__
  `3.1 <https://docs.pgrouting.org/3.1/en/pgr_primBFS.html>`__
  `3.0 <https://docs.pgrouting.org/3.0/en/pgr_primBFS.html>`__

pgr_primBFS
===============================================================================

``pgr_primBFS`` — Prim's algorithm for Minimum Spanning Tree with Depth First
Search ordering.

.. figure:: images/boost-inside.jpeg
   :target: https://www.boost.org/libs/graph/doc/prim_minimum_spanning_tree.html

   Boost Graph Inside

.. rubric:: Availability

* Version 3.0.0

  * New **Official** function


Description
-------------------------------------------------------------------------------

Visits and extracts the nodes information in Breath First Search ordering
of the Minimum Spanning Tree created with Prims's algorithm.

**The main Characteristics are:**

.. include:: prim-family.rst
   :start-after: prim-description-start
   :end-before: prim-description-end

- Returned tree nodes from a root vertex are on Breath First Search order
- Breath First Search Running time: :math:`O(E + V)`

Signatures
-------------------------------------------------------------------------------

.. code-block:: none

    pgr_primBFS(Edges SQL, Root vid [, max_depth])
    pgr_primBFS(Edges SQL, Root vids [, max_depth])

    RETURNS SET OF (seq, depth, start_vid, node, edge, cost, agg_cost)

.. index::
    single: primBFS(Single vertex)

Single vertex
...............................................................................

.. code-block:: none

    pgr_primBFS(Edges SQL, Root vid [, max_depth])

    RETURNS SET OF (seq, depth, start_vid, node, edge, cost, agg_cost)

:Example: The Minimum Spanning Tree having as root vertex :math:`2`

.. literalinclude:: doc-pgr_primBFS.queries
   :start-after: --q1
   :end-before: --q2

.. index::
    single: primBFS(Multiple vertices)

Multiple vertices
...............................................................................

.. code-block:: none

    pgr_primBFS(Edges SQL, Root vids [, max_depth])

    RETURNS SET OF (seq, depth, start_vid, node, edge, cost, agg_cost)

:Example: The Minimum Spanning Tree starting on vertices :math:`\{13, 2\}` with :math:`depth <= 3`

.. literalinclude:: doc-pgr_primBFS.queries
   :start-after: --q2
   :end-before: --q3

.. Parameters, Inner query & result columns
   are the same as kruskal

.. include:: pgr_kruskalDFS.rst
   :start-after: mstfs-information-start
   :end-before: mstfs-information-end


See Also
-------------------------------------------------------------------------------

* :doc:`spanningTree-family`
* :doc:`prim-family`
* The queries use the :doc:`sampledata` network.
* `Boost: Prim's algorithm documentation <https://www.boost.org/libs/graph/doc/prim_minimum_spanning_tree.html>`__
* `Wikipedia: Prim's algorithm <https://en.wikipedia.org/wiki/Prim%27s_algorithm>`__

.. rubric:: Indices and tables

* :ref:`genindex`
* :ref:`search`
