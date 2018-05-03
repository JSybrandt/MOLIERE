# Visualization

This sub-project creates a simple interactive HTML page from the results of a MOLIERE query.
Highlighted in this visualization is the implicit connection between topics and the most relevant papers per topic.
Additionally, this project is responsible for converting the moliere output to a standard json format.

My logic here is to try and separate the inner-workings of moliere from whatever web interface may try to access it.
And, as a research project, the format of each moliere output file has the potential to change with little notice.
The json api, on the other hand, should absorb these changes from any end-user.
