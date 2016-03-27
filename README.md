# File-management
C++ code to add, update and delete records in a file by efficiently managing the deleted memory for re-allocation

Operations:
  (i) Add
  (ii) Update
  (iii) Delete

The code dynamically manages the deleted space by reclaiming it for use by newly inserted records. 
Strategies used for keeping track of the deleted space:
  (i) First fit
  (ii) Best fit
  (iii) Worst fit
  
Compilation command: g++ assn_2.cpp -o assn_2
Executing the code: ./assn_2 --first-fit file_name.txt

Note: --first-fit can be replaced by --best-fit or --worst-fit

