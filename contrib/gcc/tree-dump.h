/* Tree-dumping functionality for intermediate representation.
   Copyright (C) 1999, 2000, 2003 Free Software Foundation, Inc.
   Written by Mark Mitchell <mark@codesourcery.com>

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

#ifndef GCC_TREE_DUMP_H
#define GCC_TREE_DUMP_H

/* Flags used with queue functions.  */
#define DUMP_NONE     0
#define DUMP_BINFO    1

/* Information about a node to be dumped.  */

typedef struct dump_node_info
{
  /* The index for the node.  */
  unsigned int index;
  /* Nonzero if the node is a binfo.  */
  unsigned int binfo_p : 1;
} *dump_node_info_p;

/* A dump_queue is a link in the queue of things to be dumped.  */

typedef struct dump_queue
{
  /* The queued tree node.  */
  splay_tree_node node;
  /* The next node in the queue.  */
  struct dump_queue *next;
} *dump_queue_p;

/* A dump_info gives information about how we should perform the dump
   and about the current state of the dump.  */

struct dump_info
{
  /* The stream on which to dump the information.  */
  FILE *stream;
  /* The original node.  */
  tree node;
  /* User flags.  */
  int flags;
  /* The next unused node index.  */
  unsigned int index;
  /* The next column.  */
  unsigned int column;
  /* The first node in the queue of nodes to be written out.  */
  dump_queue_p queue;
  /* The last node in the queue.  */
  dump_queue_p queue_end;
  /* Free queue nodes.  */
  dump_queue_p free_list;
  /* The tree nodes which we have already written out.  The
     keys are the addresses of the nodes; the values are the integer
     indices we assigned them.  */
  splay_tree nodes;
};

/* Dump the CHILD and its children.  */
#define dump_child(field, child) \
  queue_and_dump_index (di, field, child, DUMP_NONE)

extern void dump_pointer (dump_info_p, const char *, void *);
extern void dump_int (dump_info_p, const char *, int);
extern void dump_string (dump_info_p, const char *);
extern void dump_stmt (dump_info_p, tree);
extern void dump_next_stmt (dump_info_p, tree);
extern void queue_and_dump_index (dump_info_p, const char *, tree, int);
extern void queue_and_dump_type (dump_info_p, tree);

#endif /* ! GCC_TREE_DUMP_H */
