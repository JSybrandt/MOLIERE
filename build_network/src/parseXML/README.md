Parse XML
=========


This subproject uses the pugi xml library (included in the external dir) to parse medline xml files.
This code is single threaded and runs on one file at a time.
The intention is to spawn multiple instances of this code to process large data sets.
