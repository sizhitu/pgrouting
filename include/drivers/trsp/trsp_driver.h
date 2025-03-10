/*PGR-GNU*****************************************************************
File: trsp_driver.h

Shortest path with turn restrictions algorithm for PostgreSQL

Copyright (c) 2017 pgRouting developers
Copyright (c) 2011 Stephen Woodbridge

-------------

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

#ifndef INCLUDE_DRIVERS_TRSP_TRSP_DRIVER_H_
#define INCLUDE_DRIVERS_TRSP_TRSP_DRIVER_H_

/* for size_t and int64_t */
#ifdef __cplusplus
#  include <cstddef>
#  include <cstdint>
using Restriction_t = struct Restriction_t;
using Path_rt = struct Path_rt;
using Edge_t = struct Edge_t;
#else
#  include <stddef.h>
#  include <stdint.h>
typedef struct Restriction_t Restriction_t;
typedef struct Path_rt Path_rt;
typedef struct Edge_t Edge_t;
#endif


#ifdef __cplusplus
extern "C" {
#endif

    void do_trsp(
            Edge_t *edges,
            size_t edge_count,

            Restriction_t *restrictions,
            size_t restrictions_size,

            int64_t  *start_vidsArr,
            size_t size_start_vidsArr,

            int64_t  *end_vidsArr,
            size_t size_end_vidsArr,

            bool directed,

            Path_rt **return_tuples,
            size_t *return_count,

            char **log_msg,
            char **notice_msg,
            char **err_msg);


#ifdef __cplusplus
}
#endif

#endif  // INCLUDE_DRIVERS_TRSP_TRSP_DRIVER_H_
