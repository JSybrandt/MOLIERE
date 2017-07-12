#!/home/jsybran/bin/julia-v0.6/bin/julia -p 24
#=
This will cluster hypotheses based on their set similarities
=#


using ArgParse
using Base.Filesystem

function main()
  s = ArgParseSettings()
  @add_arg_table s begin
    "--cluster-data-file", "-c"
      help = "Path to read resulting matrix factors and cluster data."
      arg_type = String
    "--view-dir", "-d"
      help = "directory containing all hypotheses"
      arg_type = String
    "--verbose", "-v"
      action = :store_true
  end

  args = parse_args(ARGS, s)

  clusterDataPath = args["cluster-data-file"]
  verbose = args["verbose"]
  viewDirPath = args["view-dir"]

  file = open(clusterDataPath)
  clusters, W, H = deserialize(file)
  close(file)

  numClusters = size(W, 2)

  function getClusterDirPath(clusterId)
    return joinpath(viewDirPath, string("cluster", clusterId))
  end
  for clusterId in 1:numClusters
    dirPath = getClusterDirPath(clusterId)
    verbose && println("mkdir ", dirPath)
    mkpath(dirPath)
  end
  for rowIdx in 1:size(clusters, 1)
    fileName = clusters[rowIdx, 1]
    clusterId = clusters[rowIdx, 2]
    src = joinpath(viewDirPath, fileName)
    dest = joinpath(getClusterDirPath(clusterId), fileName)
    if isfile(src) && !isfile(dest)
      verbose && println("mv ", src, " ", dest)
      mv(src, dest)
    else
      error("FAILED TO MOVE", src)
      exit(1)
    end
  end
end

main()
