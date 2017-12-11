var NAVTREE =
[
  [ "HTGS", "index.html", [
    [ "Hybrid Task Graph Scheduler (HTGS) - An application programming interface to generate hybrid pipeline workflow systems", "index.html", [
      [ "Installation ", "index.html#installation", null ],
      [ "Approach ", "index.html#approach", null ],
      [ "HTGS Design Methodology ", "index.html#methodlogy", null ],
      [ "Overview of HTGS ", "index.html#overview", null ],
      [ "Tutorials ", "index.html#tutorials", null ]
    ] ],
    [ "Getting Started", "install-instructions.html", null ],
    [ "Tutorial 0", "tutorial0.html", [
      [ "Downloading the Tutorials ", "tutorial0.html#tut0-download", null ],
      [ "Getting Started using the Command-Line ", "tutorial0.html#tut0-cmd", null ],
      [ "Getting Started using Eclipse CDT ", "tutorial0.html#tut0-eclipse", null ],
      [ "Getting Started using CLion ", "tutorial0.html#tut0-clion", null ]
    ] ],
    [ "Tutorial 1", "tutorial1.html", [
      [ "Objectives ", "tutorial1.html#tut1-objectives", null ],
      [ "API Used ", "tutorial1.html#tut1-api-used", null ],
      [ "Implementation ", "tutorial1.html#tut1-implementation", [
        [ "Data", "tutorial1.html#tut1-data", [
          [ "Input data implementation", "tutorial1.html#tut1-input-data", null ],
          [ "Output data implementation", "tutorial1.html#tut1-output-data", null ],
          [ "Notes", "tutorial1.html#tut1-data-notes", null ]
        ] ],
        [ "Tasks", "tutorial1.html#tut1-tasks", [
          [ "AddTask Implementation", "tutorial1.html#tut1-addtask-implementation", null ],
          [ "Notes", "tutorial1.html#tut1-task-notes", null ]
        ] ],
        [ "Creating and Executing the htgs::TaskGraphConf", "tutorial1.html#tut1-create-and-execute-taskgraph", [
          [ "Main function (create and execute TaskGraph)", "tutorial1.html#tut1-main-function", null ],
          [ "Debugging and profiling a htgs::TaskGraphConf", "tutorial1.html#tut1-debug", null ],
          [ "Notes", "tutorial1.html#taskgraph-notes", null ]
        ] ]
      ] ],
      [ "Summary ", "tutorial1.html#summary", null ]
    ] ],
    [ "Tutorial 2A", "tutorial2a.html", [
      [ "Objectives ", "tutorial2a.html#tut2a-objectives", null ],
      [ "API Used ", "tutorial2a.html#tut2a-api-used", null ],
      [ "Implementation ", "tutorial2a.html#tut2a-implementation", [
        [ "Data", "tutorial2a.html#tut2a-data", [
          [ "MatrixRequestData", "tutorial2a.html#tut2a-matrix-request", null ],
          [ "MatrixBlockData", "tutorial2a.html#tut2a-matrix-data", null ],
          [ "MatrixBlockMulData", "tutorial2a.html#tut2a-matrix-mul-data", null ],
          [ "Notes", "tutorial2a.html#tut2a-data-notes", null ]
        ] ],
        [ "Tasks", "tutorial2a.html#tut2a-tasks", [
          [ "GenMatrixTask", "tutorial2a.html#tut2a-gen-matrix-task", null ],
          [ "HadamardProductTask", "tutorial2a.html#tut2a-hadamard-product-task", null ],
          [ "Notes", "tutorial2a.html#tut2a-task-notes", null ]
        ] ],
        [ "Managing Dependencies with the Bookkeeper and IRule", "tutorial2a.html#tut2a-bookkeeper", [
          [ "HadamardLoadRule", "tutorial2a.html#tut2a-load-rule", null ],
          [ "Notes", "tutorial2a.html#tut2a-bookkeeper-notes", null ]
        ] ],
        [ "Creating and Executing the htgs::TaskGraphConf", "tutorial2a.html#tut2a-create-and-execute-taskgraph", [
          [ "Main function (Hadamard Product)", "tutorial2a.html#tut2a-main-function", null ],
          [ "Notes", "tutorial2a.html#tut2a-taskgraph-notes", null ]
        ] ]
      ] ],
      [ "Summary ", "tutorial2a.html#tut2a-summary", null ]
    ] ],
    [ "Tutorial 2B", "tutorial2b.html", [
      [ "Objectives ", "tutorial2b.html#tut2b-objectives", null ],
      [ "API Used ", "tutorial2b.html#tut2b-api-used", null ],
      [ "Implementation ", "tutorial2b.html#tut2b-implementation", [
        [ "Data", "tutorial2b.html#tut2b-data", null ],
        [ "Tasks", "tutorial2b.html#tut2b-tasks", [
          [ "ReadDiskMatrixTask", "tutorial2b.html#tut2b-read-matrix-task", null ],
          [ "HadamardProductTaskWithReleaseMem", "tutorial2b.html#tut2b-hadamard-product-task", null ],
          [ "Notes", "tutorial2b.html#tut2b-task-notes", null ]
        ] ],
        [ "Managing Dependencies with a Bookkeeper and IRules", "tutorial2b.html#tut2b-bookkeeper", null ],
        [ "Throttling Tasks with a Memory Manager", "tutorial2b.html#tut2b-memory-manager", [
          [ "Static Memory Manager", "tutorial2b.html#tut2b-static-mm", null ],
          [ "Dynamic Memory Manager", "tutorial2b.html#tut2b-dynamic-mm", null ],
          [ "MatrixAllocator", "tutorial2b.html#tut2b-matrix-allocator", null ],
          [ "MatrixMemoryRule", "tutorial2b.html#tut2b-matrix-memory-rule", null ],
          [ "Notes", "tutorial2b.html#tut2b-memorymanagement-notes", null ]
        ] ],
        [ "Creating and Executing the TaskGraph", "tutorial2b.html#tut2b-create-and-execute-taskgraph", [
          [ "Main function (Hadamard Product)", "tutorial2b.html#tut2b-main-function", null ],
          [ "Notes", "tutorial2b.html#tut2b-taskgraph-notes", null ]
        ] ]
      ] ],
      [ "Summary ", "tutorial2b.html#tut2b-summary", null ]
    ] ],
    [ "Tutorial 3A", "tutorial3a.html", [
      [ "Objectives ", "tutorial3a.html#tut3a-objectives", null ],
      [ "API Used ", "tutorial3a.html#tut3a-api-used", null ],
      [ "Implementation ", "tutorial3a.html#tut3a-implementation", [
        [ "Data", "tutorial3a.html#tut3a-data", null ],
        [ "Tasks", "tutorial3a.html#tut3a-tasks", [
          [ "LoadMatrixTask", "tutorial3a.html#tut3a-load-matrix-task", null ],
          [ "MatMulBlkTask", "tutorial3a.html#tut3a-matmul-task", null ],
          [ "MatMulAccumTask", "tutorial3a.html#tut3a-matmul-accum-task", null ],
          [ "MatMulOutputTask", "tutorial3a.html#tut3a-matmul-output-task", null ],
          [ "Notes", "tutorial3a.html#tut3a-task-notes", null ]
        ] ],
        [ "Managing Dependencies with the Bookkeeper and IRule", "tutorial3a.html#tut3a-bookkeeper", [
          [ "MatMulDistributeRule", "tutorial3a.html#tut3a-distr-rule", null ],
          [ "MatMulLoadRule", "tutorial3a.html#tut3a-load-rule", null ],
          [ "MatMulAccumulateRule", "tutorial3a.html#tut3a-acc-rule", null ],
          [ "MatMulOutputRule", "tutorial3a.html#tut3a-output-rule", null ],
          [ "Notes", "tutorial3a.html#tut3a-bookkeeper-notes", null ]
        ] ],
        [ "Creating and Executing the htgs::TaskGraphConf", "tutorial3a.html#tut3a-create-and-execute-taskgraph", [
          [ "Main function (Matrix Multiplication)", "tutorial3a.html#tut3a-main-function", null ],
          [ "Notes", "tutorial3a.html#tut3a-taskgraph-notes", null ]
        ] ]
      ] ],
      [ "Summary ", "tutorial3a.html#tut3a-summary", null ]
    ] ],
    [ "Tutorial 3B", "tutorial3b.html", [
      [ "Objectives ", "tutorial3b.html#tut3b-objectives", null ],
      [ "API Used ", "tutorial3b.html#tut3b-api-used", [
        [ "Implementation, Data, Tasks, and Dependencies", "tutorial3b.html#tut3b-imp-data-tasks-dep", null ],
        [ "Debugging and Profiling htgs::TaskGraphConf", "tutorial3b.html#tut3b-debug-taskgraph", [
          [ "Visualizing Before Executing a htgs::TaskGraphConf", "tutorial3b.html#tut3b-vis-before", null ],
          [ "Visualizing After Executing a htgs::TaskGraphConf", "tutorial3b.html#tut3b-vis-after", null ]
        ] ],
        [ "Improving Utilization of Matrix Multiplication in HTGS", "tutorial3b.html#tut3b-optimize", [
          [ "Notes", "tutorial3b.html#tut3b-taskgraph-notes", null ]
        ] ]
      ] ],
      [ "Summary ", "tutorial3b.html#tut3b-summary", null ]
    ] ],
    [ "Tutorial 4", "tutorial4.html", [
      [ "Objectives ", "tutorial4.html#tut4-objectives", null ],
      [ "API Used ", "tutorial4.html#tut4-api-used", null ],
      [ "Implementation ", "tutorial4.html#tut4-implementation", [
        [ "Data", "tutorial4.html#tut4-data", null ],
        [ "Tasks", "tutorial4.html#tut4-tasks", [
          [ "LoadMatrixTask", "tutorial4.html#tut4-load-matrix-task", null ],
          [ "MatMulBlkTask", "tutorial4.html#tut4-matmul-task", null ],
          [ "MatMulAccumTask", "tutorial4.html#tut4-matmul-accum-task", null ],
          [ "MatMulOutputTask", "tutorial4.html#tut4-matmul-output-task", null ],
          [ "Notes", "tutorial4.html#tut4-task-notes", null ]
        ] ],
        [ "Managing Dependencies with the Bookkeeper and IRule", "tutorial4.html#tut4-bookkeeper", [
          [ "MatMulDistributeRule", "tutorial4.html#tut4-distr-rule", null ],
          [ "MatMulLoadRule", "tutorial4.html#tut4-load-rule", null ],
          [ "MatMulAccumulateRule", "tutorial4.html#tut4-acc-rule", null ],
          [ "MatMulOutputRule", "tutorial4.html#tut4-output-rule", null ],
          [ "Notes", "tutorial4.html#tut4-bookkeeper-notes", null ]
        ] ],
        [ "Creating and Executing the htgs::TaskGraphConf", "tutorial4.html#tut4-create-and-execute-taskgraph", [
          [ "Main function (Matrix Multiplication)", "tutorial4.html#tut4-main-function", null ],
          [ "Notes", "tutorial4.html#tut4-taskgraph-notes", null ]
        ] ]
      ] ],
      [ "Summary ", "tutorial4.html#tut4-summary", null ]
    ] ],
    [ "Namespaces", null, [
      [ "Namespace List", "namespaces.html", "namespaces" ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", "functions_func" ],
        [ "Variables", "functions_vars.html", null ],
        [ "Related Functions", "functions_rela.html", null ]
      ] ]
    ] ],
    [ "Files", null, [
      [ "File List", "files.html", "files" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
".html",
"classhtgs_1_1_any_task_graph_conf.html#aab64ec2f1dfebbea602daeb0280c5030",
"classhtgs_1_1_i_task.html#a5a5b471973ab49fc6a9cfcfbe4dacaa4",
"classhtgs_1_1_task_graph_profiler.html#ab0f6d742dead95967793f90437b3104c",
"tutorial4.html#tut4-acc-rule"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';