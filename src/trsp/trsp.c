/*PGR-GNU*****************************************************************

File: trsp.c

Generated with Template by:
Copyright (c) 2013 pgRouting developers
Mail: project@pgrouting.org

------

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

 ********************************************************************PGR-GNU*/


#include "c_common/postgres_connection.h"
#include "catalog/pg_type.h"
#include "c_common/debug_macro.h"


#include "c_types/trsp/trsp.h"
#include "c_types/edge_t.h"

#include "c_common/edges_input.h"

#include "trsp/trsp_core.h"
typedef struct restrict_t restrict_t;
typedef struct Edge_t Edge_t;
typedef struct path_element_tt path_element_tt;

PGDLLEXPORT Datum _pgr_trsp(PG_FUNCTION_ARGS);


typedef struct edge_columns {
  int id;
  int source;
  int target;
  int cost;
  int reverse_cost;
} edge_columns_t;

typedef struct restrict_columns {
  int target_id;
  int via_path;
  int to_cost;
} restrict_columns_t;


/*
 * This function fetches the resturction columns from an SPITupleTable..
 *
*/
static int
fetch_restrict_columns(SPITupleTable *tuptable,
                       restrict_columns_t *restrict_columns) {
  restrict_columns->target_id = SPI_fnumber(tuptable->tupdesc, "target_id");
  restrict_columns->via_path = SPI_fnumber(tuptable->tupdesc, "via_path");
  restrict_columns->to_cost =  SPI_fnumber(tuptable->tupdesc, "to_cost");
  if (restrict_columns->target_id == SPI_ERROR_NOATTRIBUTE ||
      restrict_columns->via_path == SPI_ERROR_NOATTRIBUTE ||
      restrict_columns->to_cost == SPI_ERROR_NOATTRIBUTE) {
    elog(ERROR, "Error, restriction query must return columns "
        "'target_id', 'via_path' and 'to_cost'");
    return -1;
  }

  if (SPI_gettypeid(tuptable->tupdesc,
       restrict_columns->target_id) != INT4OID ||
     SPI_gettypeid(tuptable->tupdesc, restrict_columns->via_path) != TEXTOID ||
     SPI_gettypeid(tuptable->tupdesc, restrict_columns->to_cost) != FLOAT8OID) {
    elog(ERROR, "Error, restriction columns 'target_id' must be of type int4,"
     "'via_path' must be of type text, 'to_cost' must be of type float8");
    return -1;
  }

  return 0;
}


/*
 * To fetch a edge from Tuple.
 *
 */

static void
fetch_restrict(HeapTuple *tuple, TupleDesc *tupdesc,
           restrict_columns_t *restrict_columns, restrict_t *rest) {
  Datum binval;
  bool isnull;
  int t;

  for (t = 0; t < MAX_RULE_LENGTH; ++t)
    rest->via[t] = -1;

  binval = SPI_getbinval(*tuple, *tupdesc, restrict_columns->target_id,
       &isnull);
  if (isnull)
    elog(ERROR, "target_id contains a null value");
  rest->target_id = DatumGetInt32(binval);

  binval = SPI_getbinval(*tuple, *tupdesc, restrict_columns->to_cost, &isnull);
  if (isnull)
    elog(ERROR, "to_cost contains a null value");
  rest->to_cost = DatumGetFloat8(binval);
  char *str = DatumGetCString(SPI_getvalue(*tuple, *tupdesc,
       restrict_columns->via_path));

  // PGR_DBG("restriction: %f, %i, %s", rest->to_cost, rest->target_id, str);

  if (str != NULL) {
    int ci = 0;
    char* pch = (char *)strtok(str, " ,");

    while (pch != NULL && ci < MAX_RULE_LENGTH) {
      rest->via[ci] = atoi(pch);
      // PGR_DBG("    rest->via[%i]=%i", ci, rest->via[ci]);
      ci++;
      pch = (char *)strtok(NULL, " ,");
    }
  }
}



static int compute_trsp(
        char* sql,
        int dovertex,
        int64_t start_id,
        double start_pos,
        int64_t end_id,
        double end_pos,
        bool directed,
        bool has_reverse_cost,
        char* restrict_sql,
        path_element_tt **path,
        size_t *path_count) {
  pgr_SPI_connect();

  SPIPlanPtr SPIplan;
  Portal SPIportal;

  Edge_t *edges = NULL;
  size_t total_tuples = 0;
  pgr_get_edges(sql, &edges, &total_tuples);

  // defining min and max vertex id
  int64_t v_max_id = 0;
  int64_t v_min_id = INT_MAX;

  for (size_t z = 0; z < total_tuples; z++) {
    PGR_DBG("id %ld source %ld target %ld cost %f rev %f",
            edges[z].id, edges[z].source, edges[z].target, edges[z].cost, edges[z].reverse_cost);
    if (edges[z].source < v_min_id)
      v_min_id = edges[z].source;

    if (edges[z].source > v_max_id)
      v_max_id = edges[z].source;

    if (edges[z].target < v_min_id)
      v_min_id = edges[z].target;

    if (edges[z].target > v_max_id)
      v_max_id = edges[z].target;
  }

  // ::::::::::::::::::::::::::::::::::::
  // :: reducing vertex id (renumbering)
  // ::::::::::::::::::::::::::::::::::::
  /* track if start and end are both in edge tuples */
  int s_count = 0;
  int t_count = 0;
  for (size_t z = 0; z < total_tuples; z++) {
    // check if edges[] contains source and target
    if (dovertex) {
        if (edges[z].source == start_id || edges[z].target == start_id)
          ++s_count;
        if (edges[z].source == end_id || edges[z].target == end_id)
          ++t_count;
    } else {
        if (edges[z].id == start_id)
          ++s_count;
        if (edges[z].id == end_id)
          ++t_count;
    }

    edges[z].source -= v_min_id;
    edges[z].target -= v_min_id;
  }

  PGR_DBG("Min vertex id: %ld , Max vid: %ld", v_min_id, v_max_id);
  PGR_DBG("Total %ld edge tuples", total_tuples);

  if (s_count == 0) {
    elog(ERROR, "Start id was not found.");
    return -1;
  }

  if (t_count == 0) {
    elog(ERROR, "Target id was not found.");
    return -1;
  }

  if (dovertex) {
      start_id -= v_min_id;
      end_id   -= v_min_id;
  }

  PGR_DBG("Fetching restriction tuples\n");
  restrict_t *restricts = NULL;
  uint64_t total_restrict_tuples = 0;
  restrict_columns_t restrict_columns = {.target_id = -1, .via_path = -1,
                                 .to_cost = -1};

  char *err_msg;
  int ret = -1;

  if (restrict_sql == NULL) {
      PGR_DBG("Sql for restrictions is null.");
  } else {
      uint32_t TUPLIMIT = 1000;

      SPIplan = SPI_prepare(restrict_sql, 0, NULL);
      if (SPIplan  == NULL) {
          elog(ERROR, "turn_restrict_shortest_path: "
          "couldn't create query plan via SPI");
          return -1;
      }

      if ((SPIportal = SPI_cursor_open(NULL, SPIplan, NULL, NULL, true)) \
       == NULL) {
          elog(ERROR, "turn_restrict_shortest_path:"
          " SPI_cursor_open('%s') returns NULL", restrict_sql);
          return -1;
      }

      bool moredata = true;
      while (moredata == true) {
          SPI_cursor_fetch(SPIportal, true, TUPLIMIT);

          if (restrict_columns.target_id == -1) {
              if (fetch_restrict_columns(SPI_tuptable, &restrict_columns) \
              == -1) {
                PGR_DBG("fetch_restrict_columns failed!");
                pgr_SPI_finish();
                return -1;
              }
          }

          /* Suppress the -Wconversion warning temporarily */
          uint64_t ntuples = SPI_processed;

          total_restrict_tuples += ntuples;

          if (ntuples > 0) {
              if (!restricts)
                restricts = palloc(total_restrict_tuples * sizeof(restrict_t));
              else
                restricts = repalloc(restricts,
                    total_restrict_tuples * sizeof(restrict_t));

              if (restricts == NULL) {
                  pgr_SPI_finish();
                  elog(ERROR, "Out of memory");
                  return -1;
              }

              uint32_t t;
              SPITupleTable *tuptable = SPI_tuptable;
              TupleDesc tupdesc = SPI_tuptable->tupdesc;

              for (t = 0; t < ntuples; t++) {
                  HeapTuple tuple = tuptable->vals[t];
                  fetch_restrict(&tuple, &tupdesc, &restrict_columns,
                             &restricts[total_restrict_tuples - ntuples + t]);
              }
              SPI_freetuptable(tuptable);
          } else {
              moredata = false;
          }
      }
      SPI_cursor_close(SPIportal);
  }

  PGR_DBG("Total %ld restriction tuples", total_restrict_tuples);

      PGR_DBG("Calling trsp_edge_wrapper\n");
      ret = trsp_edge_wrapper(edges, total_tuples,
                        restricts, total_restrict_tuples,
                        start_id, start_pos, end_id, end_pos,
                        directed, has_reverse_cost,
                        path, path_count, &err_msg);

  PGR_DBG("Message received from inside:");
  PGR_DBG("%s", err_msg);


  // ::::::::::::::::::::::::::::::::
  // :: restoring original vertex id
  // ::::::::::::::::::::::::::::::::
  for (size_t z = 0; z < *path_count; z++) {
    if (z || (*path)[z].vertex_id != -1)
        (*path)[z].vertex_id += v_min_id;
  }

  PGR_DBG("ret = %i\n", ret);

  PGR_DBG("*path_count = %ld\n", *path_count);

  if (ret < 0) {
      ereport(ERROR, (errcode(ERRCODE_E_R_E_CONTAINING_SQL_NOT_PERMITTED),
        errmsg("Error computing path: %s", err_msg)));
    }

  pgr_SPI_finish();
  return 0;
}


PG_FUNCTION_INFO_V1(_pgr_trsp);
PGDLLEXPORT Datum
_pgr_trsp(PG_FUNCTION_ARGS) {
  FuncCallContext     *funcctx;
  TupleDesc            tuple_desc;
  path_element_tt      *path;

  // stuff done only on the first call of the function
  if (SRF_IS_FIRSTCALL()) {
      MemoryContext   oldcontext;
      size_t path_count = 0;
      int i;
      double s_pos;
      double e_pos;
      char *               sql;

      // create a function context for cross-call persistence
      funcctx = SRF_FIRSTCALL_INIT();

      // switch to memory context appropriate for multiple function calls
      oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

      // verify that the first 5 args are not NULL
      for (i = 0; i < 7; i++) {
        if (i == 2 || i == 4) continue;
        if (PG_ARGISNULL(i)) {
            elog(ERROR, "turn_restrict_shortest_path(): "
            "Argument %i may not be NULL", i+1);
        }
      }

      if (PG_ARGISNULL(2)) {
        s_pos = 0.5;
      } else {
        s_pos = PG_GETARG_FLOAT8(2);
        if (s_pos < 0.0) s_pos = 0.5;
        if (s_pos > 1.0) s_pos = 0.5;
      }

      if (PG_ARGISNULL(4)) {
        e_pos = 0.5;
      } else {
        e_pos = PG_GETARG_FLOAT8(4);
        if (e_pos < 0.0) e_pos = 0.5;
        if (e_pos > 1.0) e_pos = 0.5;
      }

      if (PG_ARGISNULL(7)) {
        sql = NULL;
      } else {
        sql = text_to_cstring(PG_GETARG_TEXT_P(7));
      }

      PGR_DBG("Calling compute_trsp");

         compute_trsp(text_to_cstring(PG_GETARG_TEXT_P(0)),
                                   0,  // sdo edge
                                   PG_GETARG_INT32(1),
                                   s_pos,
                                   PG_GETARG_INT32(3),
                                   e_pos,
                                   PG_GETARG_BOOL(5),
                                   PG_GETARG_BOOL(6),
                                   sql,
                                   &path, &path_count);

      // total number of tuples to be returned
        funcctx->max_calls = path_count;

        funcctx->user_fctx = path;
        if (get_call_result_type(fcinfo, NULL, &tuple_desc)
                != TYPEFUNC_COMPOSITE) {
            ereport(ERROR,
                    (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("function returning record called in context "
                         "that cannot accept type record")));
        }

      funcctx->tuple_desc = tuple_desc;

      MemoryContextSwitchTo(oldcontext);
    }

  // stuff done on every call of the function
  funcctx = SRF_PERCALL_SETUP();

  tuple_desc = funcctx->tuple_desc;
  path = (path_element_tt*) funcctx->user_fctx;

  if (funcctx->call_cntr < funcctx->max_calls) {
      // do when there is more left to send
      HeapTuple    tuple;
      Datum        result;
      Datum *values;
      bool* nulls;

      values = palloc(4 * sizeof(Datum));
      nulls = palloc(4 * sizeof(char));

      values[0] = Int32GetDatum(funcctx->call_cntr);
      nulls[0] = false;
      values[1] = Int32GetDatum(path[funcctx->call_cntr].vertex_id);
      nulls[1] = false;
      values[2] = Int32GetDatum(path[funcctx->call_cntr].edge_id);
      nulls[2] = false;
      values[3] = Float8GetDatum(path[funcctx->call_cntr].cost);
      nulls[3] = false;

      tuple = heap_form_tuple(tuple_desc, values, nulls);

      // make the tuple into a datum
      result = HeapTupleGetDatum(tuple);

      // clean up (this is not really necessary)
      pfree(values);
      pfree(nulls);

      SRF_RETURN_NEXT(funcctx, result);
    } else {  // do when there is no more left
      PGR_DBG("Going to free path");
      if (path) free(path);
      SRF_RETURN_DONE(funcctx);
    }
}
