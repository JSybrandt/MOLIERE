$( document ).ready(function() {

  buildGraph();
  buildTable();
  buildEval();
});

var PAPERS_FILE = "papers.json"
var NET_FILE = "network.json";
var TOPIC_FILE = "topics.json";
var EVAL_FILE = "eval.json";

focusTopic = function(topicName){
  //remove stuff inside div

  d3.json(TOPIC_FILE, function(error, topicRec){
    if (error) throw error;
    for (idx in topicRec){
      if(topicRec[idx].name === topicName){
        var topicDeltailDiv = d3.select("#topic-details");
        topicDeltailDiv.selectAll("table").remove();
        topicDeltailDiv.selectAll('h3').remove();
        var detailsTable = topicDeltailDiv.append("table")
          , thead = detailsTable.append('thead')
          , tbody = detailsTable.append('tbody');


        thead.append('tr').append('th')
          .attr('colspan', '2')
            .text(topicName);

        var header = thead.append('tr');
        header.classed('table-sub-header', true);
        header.append('th')
          .text("Word");
        header.append('th')
          .text("Count");

        var terms = topicRec[idx].children;
        console.log(terms)
        for (childIdx in terms){
          var row = tbody.append('tr');
          row.append('td').text(terms[childIdx].name);
          row.append('td').text(terms[childIdx].size);
        }
        break;
      }
    }
  });
}


buildEval = function(){
  var evalTabel = d3.select("#eval").append('table')
    , thead = evalTabel.append('thead')
    , tbody =evalTabel.append('tbody');

  d3.json(EVAL_FILE, function(error, evalData){
    if (error) throw error;
    var sortAscending = false;
    var header = thead.append('tr');
    header.classed('table-sub-header', true);
    header.append('th').text("Metric");
    header.append('th').text("Score");

    for (i in evalData){
      var row = tbody.append('tr');
      row.append('td').text(evalData[i].name);
      row.append('td').text(evalData[i].score);
    }

  });
}

buildTable = function(){

  var table = d3.select("#topics")
    , thead = table.append('thead')
    , tbody =table.append('tbody');

  d3.json(NET_FILE, function(error, netData){
    if (error) throw error;
    var nodes = netData.nodes;
    var importantTopics = []
    for ( idx in nodes ){
      if(nodes[idx].group == 2){
        importantTopics.push(nodes[idx].id);
      }
    }

    // Center Header
    thead.append('tr').append('th').attr('colspan', importantTopics.length)
         .text('Best Papers Per Topic')
    //Each Topic
    thead.append('tr')
         .selectAll('th')
         .data(importantTopics)
         .enter()
         .append('th')
           .text(function(d){return d;})


    d3.json(PAPERS_FILE, function(error, paperData){
      if (error) throw error;
      var data = [];
      var numPapersPer = paperData[importantTopics[0]].papers.length;

      for(j = 0 ; j < numPapersPer ; j++){
        var row = tbody.append('tr');
        for (i in importantTopics){
          var d = paperData[importantTopics[i]].papers[j];
          row.append('td')
             .append('a')
               .attr('href',
                     "https://www.ncbi.nlm.nih.gov/pubmed/?term="
                     + d.pmid)
               .text(d.title);
        }
      }
    });
  });
}


buildGraph = function(){

	var BUBBLE_SIZE = 80;
  var heightFract = 0.8;
  var widthFract = 0.85;

  var getWidth = function(){
      return +window.innerWidth * widthFract;
  }

  var getHeight = function(){
      return +window.innerHeight * heightFract;
  }

  // Network vis
  var netSvg = d3.select("#net"),
      format = d3.format(",d");



  var size = function(group) {
		if( group == 1 )
			return 5;
    if( group == 2 )
			return BUBBLE_SIZE;
    if( group == 3 )
			return BUBBLE_SIZE/2;
  }

  var simulation = d3.forceSimulation()
      .force("link", d3.forceLink().id(function(d) { return d.id; })
                                   .distance(function(d){ return 10; })
            )
      .force("charge", d3.forceManyBody().strength(function(d){
        if(d.group != 1)
          return -50;
        else
          return -15;
      }))
      .force("center", d3.forceCenter(getWidth()/2, getHeight()/2))
      .force('collision', d3.forceCollide()
                            .radius(function(d){return size(d.group) + 2;})
                            .strength(.9));

  d3.select(window)
    .on('resize', function(){
      netSvg.attr('width', getWidth())
            .attr('height', getHeight())
      simulation.force('center').x(getWidth() / 2)
                                .y(getHeight() / 2)
    });

  function dragstarted(d) {
    if (!d3.event.active) simulation.alphaTarget(0.3).restart();
    d.fx = d.x;
    d.fy = d.y;
    d.fixed=true;
  }

  function dragged(d) {
    d.fx = d3.event.x;
    d.fy = d3.event.y;
  }

  function dragended(d) {
    if (!d3.event.active) simulation.alphaTarget(0);
  }

  var pack = d3.pack().size([2*BUBBLE_SIZE	, 2*BUBBLE_SIZE]);


	// load topic bubble, later the graph will move them around
  d3.json(TOPIC_FILE, function(error, topicList){
    if (error) throw error;
		var topic2data = {};
		for( idx in topicList ){
      var name = topicList[idx]['name'];
			topic2data[name] = topicList[idx];
      topic2data[name].children = topic2data[name].children.slice(0, 8);
		}

    d3.json(NET_FILE, function(error, graph) {
      if (error) throw error;

      var topic2group = {};
      for (idx in graph.nodes){
        var d = graph.nodes[idx];
        topic2group[d.id] = d.group;
      }

      var link = netSvg.append("g")
          .attr("class", "links")
          .selectAll("line")
          .data(graph.links)
          .enter().append("line")
          .attr("stroke-width", function(d) {
            var a = topic2group[d.source];
            var b = topic2group[d.target];
            if( a !== 1 && b !== 1)
              return 10;
            return 2;
          })
          .attr("stroke", function(d) {
            var a = topic2group[d.source];
            var b = topic2group[d.target];
            if( a !== 1 && b !== 1)
              return "#f00";
            return "#999";
          })

      var node = netSvg.append("g")
          .attr("class", "nodes")
          .selectAll("circle")
          .data(graph.nodes)
          .enter().append("g")
          .attr('id', function(d){ return d.id;})
          .attr('class', 'class-node')
          .append("circle")
          .attr("r", function(d) { return size(d.group); })
          .attr("fill", function(d) { return colorByGroup(d.group); })
          .call(d3.drag()
                .on("start", dragstarted)
                .on("drag", dragged)
                .on("end", dragended))

          .on("click", function(d){ focusTopic(d.id);});

      node.append("title").text(function(d) { return d.id; });


      var first = true;
      //make the topic nodes bubble charts
			node.filter(function(d){ return d.group == 2; })
          .attr("class", "topic_circle")
          .each(function(d){
            var g = d3.select('#'+d.id);

            // focus one of the d2 nodes auto
            if (first){
              first = false;
              focusTopic(d.id);
            }

            g.append('text')
              .attr('class', 'topicTag')
              .attr("transform", function(d) {
                return "translate(0," + (-BUBBLE_SIZE) + ")";
              })
              .text(function(d){return d.id;});

            var data = topic2data[d.id];
            var root = d3.hierarchy(data)
              .sum(function(d) { return d.size; })
              .sort(function(a, b) { return b.value - a.value; });
            var subNode = g.selectAll(".node")
              .data(pack(root).descendants())
              .enter()
              .filter(function(d){ return !d.children; })
              .append('g')
              .attr("class",  "leaf node")
              .attr("transform", function(d) {
                return "translate(" + (d.x - BUBBLE_SIZE) + "," + (d.y - BUBBLE_SIZE) + ")";
              });
            subNode.append("title")
              .text(function(d) {
                return d.data.name + "\n" + format(d.value);
              });

            subNode.append("circle")
              .attr("r", function(d) { return d.r; });

            subNode.filter(function(d) { return !d.children; }).append("text")
              .attr("dy", "0.3em")
              // .text(function(d) { return d.data.name.substring(0, d.r / 3); });
              .text(function(d){ return d.data.name; })
          });

      node.filter(function(d){ return d.group == 3; })
          .each(function(d) {
            var n = d3.select("#"+d.id);
            n.append('text')
             .attr("dy", "0.3em")
             .text(function(d) { return d.id; });
          })

      simulation
        .nodes(graph.nodes)
        .on("tick", ticked);

      simulation.force("link")
        .links(graph.links);

      function ticked() {
        link
            .attr("x1", function(d) { return d.source.x; })
            .attr("y1", function(d) { return d.source.y; })
            .attr("x2", function(d) { return d.target.x; })
            .attr("y2", function(d) { return d.target.y; });

        node.each(function(d){
          d3.select("#"+d.id)
            .attr("transform", function(d) { return "translate(" + d.x + "," + d.y + ")"; });
        })
      }
    });
  });

  var colorByGroup = function(group) {
    if(group === 1){
      // Regular Topic
      return d3.rgb("#3182bd");
    } else if (group === 2){
      // Path Topic
      return d3.rgb("#e6550d");
    } else if (group === 3){
      // Keyword
      return d3.rgb("#e6550d");
    }
  }
}
