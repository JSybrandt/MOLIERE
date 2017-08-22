#!/home/jsybran/bin/julia-v0.6/bin/julia
#=
This will cluster hypotheses based on their set similarities
=#


using ArgParse
using Base.Filesystem

using Netpbm, ColorTypes, FixedPointNumbers, IndirectArrays, ImageCore, FileIO
using ImageMagick

function saveMat(filePath::AbstractString, X::Array)
  C = convert(Array{Gray{N0f16}}, X)
  outFile = FileIO.File(format"PGMBinary", filePath)
  Netpbm.save(outFile, C)
end

function loadMat(filePath::AbstractString)
  C = ImageMagick.load(filePath)
  #inFile = FileIO.File(format"PGMBinary", filePath)
  #C = Netpbm.load(inFile)
  return convert(Array{Float64}, C)
end

function main()
  s = ArgParseSettings()
  @add_arg_table s begin
    "--cluster-data-file", "-c"
      help = "Path to read resulting matrix factors and cluster data."
      arg_type = String
    "--res-matrix-file", "-m"
      help = "output matrix file which is a selection from the in-matrix-file"
      arg_type = String
    "--cluster-num", "-k"
      help = "selects which cluster data to create"
      arg_type = Int
    "--res-hypo-names", "-l"
      help = "output hyponame file"
      arg_type = String
    "--in-matrix-file", "-M"
      help = "input matrix file"
      arg_type = String
    "--verbose", "-v"
      action = :store_true
  end

  args = parse_args(ARGS, s)

  verbose = args["verbose"]
  clusterDataPath = args["cluster-data-file"]
  resMatPath = args["res-matrix-file"]
  cNum = args["cluster-num"]
  resHypoNamePath = args["res-hypo-names"]
  inMatPath = args["in-matrix-file"]

  file = open(clusterDataPath)
  clusters, W, H = deserialize(file)
  close(file)

  numClusters = size(W, 2)

  @assert cNum <= numClusters

  verbose && println("Started!")
  goodRows = find(row->clusters[row,2]==cNum, 1:size(clusters,1))

  verbose && println("Found:", size(goodRows))
  verbose && println("Writing HypoName.txt!")
  file = open(resHypoNamePath, "w")
  for row in goodRows
    write(file, string(clusters[row,1],"\n"))
  end
  close(file)

  verbose && println("Getting Mat Subset")
  verbose && println("Loading", inMatPath)
  mat = loadMat(inMatPath)
  subMat = mat[goodRows, goodRows]
  verbose && println("Writing", resMatPath)
  saveMat(resMatPath, subMat)
end

main()
