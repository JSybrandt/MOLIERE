$( document ).ready(function() {

  var NET_FILE = "network.json";
  var TOPIC_FILE = "topics.json";

	var BUBBLE_SIZE = 100;

  // Network vis
  var netSvg = d3.select("#net"),
      width = +netSvg.attr("width"),
      height = +netSvg.attr("height"),
      format = d3.format(",d");

  var color = function(group) {
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

  var size = function(group) {
		if( group == 1 )
			return 5;
    if( group == 2 )
			return BUBBLE_SIZE;
    if( group == 3 )
			return BUBBLE_SIZE/2;
  }

  var simulation = d3.forceSimulation()
      .force("link", d3.forceLink().id(function(d) { return d.id; }))
      .force("charge", d3.forceManyBody())
      .force("center", d3.forceCenter(width / 2, height / 2))
      .force('collision', d3.forceCollide().radius(BUBBLE_SIZE/2));

  function dragstarted(d) {
    if (!d3.event.active) simulation.alphaTarget(0.3).restart();
    d.fx = d.x;
    d.fy = d.y;
  }

  function dragged(d) {
    d.fx = d3.event.x;
    d.fy = d3.event.y;
  }

  function dragended(d) {
    if (!d3.event.active) simulation.alphaTarget(0);
    d.fx = null;
    d.fy = null;
  }

  var pack = d3.pack().size([2*BUBBLE_SIZE	, 2*BUBBLE_SIZE]);


	// load topic bubble, later the graph will move them around
  d3.json(TOPIC_FILE, function(error, topicList){
    if (error) throw error;
		var topic2data = {};
		for( idx in topicList ){
			topic2data[topicList[idx]['name']] = topicList[idx];
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
          .append("circle")
          .attr("r", function(d) { return size(d.group); })
          .attr("fill", function(d) { return color(d.group); })
          .call(d3.drag()
                .on("start", dragstarted)
                .on("drag", dragged)
                .on("end", dragended));

      node.append("title").text(function(d) { return d.id; });

      //make the topic nodes bubble charts
			node.filter(function(d){ return d.group == 2; })
          .attr("class", "topic_circle")
          .append('g')
          .attr('class', 'root')
          .each(function(d){
            console.log(d);
            var g = d3.select('#'+d.id);
            var data = topic2data[d.id];
            var root = d3.hierarchy(data)
              .sum(function(d) { return d.size; })
              .sort(function(a, b) { return b.value - a.value; });
            var subNode = g.selectAll(".node")
              .data(pack(root).descendants())
              .enter().append('g')
              .filter(function(d){ return !d.children; })
              .attr("class",  "leaf node")
              //.attr("class", function(d) { return d.children ? "node" : "leaf node"; })
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
              .text(function(d) { return d.data.name.substring(0, d.r / 3); });
          });

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
        //node
            //.attr("cx", function(d) { return d.x; })
            //.attr("cy", function(d) { return d.y; })
      }


    });
  });







});



/*
vg = d3.select("svg"),
    width = +svg.attr("width"),
    height = +svg.attr("height");

var format = d3.format(",d");

var color = d3.scaleSequential(d3.interpolateMagma)
    .domain([-4, 4]);

var stratify = d3.stratify()
    .parentId(function(d) { return d.id.substring(0, d.id.lastIndexOf(".")); });

var pack = d3.pack()
    .size([width - 2, height - 2])
    .padding(3);

d3.csv("flare.csv", function(error, data) {
  if (error) throw error;

  var root = stratify(data)
      .sum(function(d) { return d.value; })
      .sort(function(a, b) { return b.value - a.value; });

  pack(root);

  var node = svg.select("g")
    .selectAll("g")
    .data(root.descendants())
    .enter().append("g")
      .attr("transform", function(d) { return "translate(" + d.x + "," + d.y + ")"; })
      .attr("class", function(d) { return "node" + (!d.children ? " node--leaf" : d.depth ? "" : " node--root"); })
      .each(function(d) { d.node = this; })
      .on("mouseover", hovered(true))
      .on("mouseout", hovered(false));

  node.append("circle")
      .attr("id", function(d) { return "node-" + d.id; })
      .attr("r", function(d) { return d.r; })
      .style("fill", function(d) { return color(d.depth); });

  var leaf = node.filter(function(d) { return !d.children; });

  leaf.append("clipPath")
      .attr("id", function(d) { return "clip-" + d.id; })
    .append("use")
      .attr("xlink:href", function(d) { return "#node-" + d.id + ""; });

  leaf.append("text")
      .attr("clip-path", function(d) { return "url(#clip-" + d.id + ")"; })
    .selectAll("tspan")
    .data(function(d) { return d.id.substring(d.id.lastIndexOf(".") + 1).split(/(?=[A-Z][^A-Z])/g); })
    .enter().append("tspan")
      .attr("x", 0)
      .attr("y", function(d, i, nodes) { return 13 + (i - nodes.length / 2 - 0.5) * 10; })
      .text(function(d) { return d; });

  node.append("title")
      .text(function(d) { return d.id + "\n" + format(d.value); });
});

function hovered(hover) {
  return function(d) {
    d3.selectAll(d.ancestors().map(function(d) { return d.node; })).classed("node--hover", hover);
  };
}/

*/
