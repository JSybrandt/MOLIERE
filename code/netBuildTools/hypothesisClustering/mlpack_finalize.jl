#!/home/jsybran/bin/julia-v0.6/bin/julia

using ArgParse
using NMFk
using Netpbm, ColorTypes, FixedPointNumbers, IndirectArrays, ImageCore, FileIO

function saveMat(filePath::AbstractString, X::Array)
  C = convert(Array{Gray{N0f16}}, X)
  outFile = FileIO.File(format"PGMBinary", filePath)
  Netpbm.save(outFile, C)
end

function loadMat(filePath::AbstractString)
  inFile = FileIO.File(format"PGMBinary", filePath)
  C = Netpbm.load(inFile)
  return convert(Array{Float64}, C)
end

"Insert such that Big is always column oriented"
function insertIntoBig!(Big::Matrix{Float64}, M::Matrix{Float64}, idx::Int , k::Int)
  @assert size(Big, 1) == size(M, 1) # is a W
  @assert k == size(M, 2)
  endInx = k * idx
  startIdx = endInx - k + 1
  Big[:, startIdx:endInx] = M
end

function zeroVecFix!(factors::Vector{Matrix}; verbose::Bool=false)
  n, k = size(factors[1])
  isW = true
  if k > n
    isW = false
    n, k = (k, n)
  end

  verbose && println("Found $(length(factors)) trials with k = $k")

  # fix zero case
  needZeroFix = false
  for i in 1:length(factors)
    factor = factors[i]
    if minimum(sum(factor, (isW ? 1 : 2))) == 0  # if we have a zero column
      needZeroFix = true
      break
    end
  end

  if needZeroFix
    if isW
      verbose && println("Performing zero fix on Ws")
      biasRow = [1 for i in 1:k]'
      for i in 1:length(factors)
        factors[i] = vcat(factors[i], biasRow)
      end
    else
      verbose && println("Performing zero fix on Hs")
      biasCol = [1 for i in 1:k]
      for i in 1:length(factors)
        factors[i] = hcat(factors[i], biasCol)
      end
    end
  end
end

function undoZeroFix(factor::Matrix, X::Matrix)
  a, k = size(factor)
  n, m = size(X)
  isW = true
  if a < k
    a, k = (k, a)
    isW = false
  end
  # if the dim doesn't line up
  if (isW && n != a) || (!isW && m != a)
    # if the dim is 1 off
    if (isW && n+1 == a) || (!isW && m+1 == a)
      if isW
        return factor[1:n, :]
      else
        return factor[:, 1:m]
      end
    else
      error("undoZeroFix called with matrix of invalid size")
    end
  end
  return factor
end

function main()
  s = ArgParseSettings()
  @add_arg_table s begin
    "--result-dir", "-d"
      help = "Path to the matrix files. Matrix files must begin with W or H, and must be .pgm"
      arg_type = String
      required = true
    "--out-W", "-W"
      help = "Path to the resulting best W file .pgm"
      arg_type = String
      required = true
    "--out-H", "-H"
      help = "Path to the output H data file .pgm"
      arg_type = String
      required = true
    "--in-X", "-X"
      help = "Path to the input X matrix .pgm. Used to calc obj values"
      arg_type = String
      required = true
    "--out-cluster-data", "-C"
      help = "Path to the output cluster data"
      arg_type = String
      required = true
    "--in-index-labels", "-l"
      help = "Path to the input label file"
      arg_type = String
      required = true
    "--k", "-k"
      help = "k value for this data"
      arg_type = Int
      required = true
    "--cluster-w", "-w"
      action = :store_true
    "--verbose", "-v"
      action = :store_true
  end

  args = parse_args(ARGS, s)

  resultDirPath = args["result-dir"]
  outWPath = args["out-W"]
  outHPath = args["out-H"]
  inXPath = args["in-X"]
  labelPath = args["in-index-labels"]
  clusterDataPath = args["out-cluster-data"]
  k = args["k"]
  clusterWeights = args["cluster-w"]
  verbose = args["verbose"]

  verbose && println("Loading:", inXPath)
  X = loadMat(inXPath) # Don't need transpose, written by julia
  n, m = size(X)

  verbose && println("Processing: ", resultDirPath)

  WFiles = []
  HFiles = []

  for fileName in readdir(resultDirPath)
    if length(fileName) > 4 && fileName[end-3:end] == ".pgm"
      if fileName[1] == 'W'
        push!(WFiles, joinpath(resultDirPath, fileName))
      elseif fileName[1] == 'H'
        push!(HFiles, joinpath(resultDirPath, fileName))
      end
    end
  end

  WFiles = sort(WFiles)
  HFiles = sort(HFiles)

  if verbose
    println("WFiles / HFiles")
    println(hcat(WFiles[:], HFiles[:]))
  end


  @assert length(WFiles) == length(HFiles)
  numTrials = length(WFiles)
  for i in 1:numTrials
    # assert that the files have been paired properly.
  end

  verbose && println("Identified trials:", numTrials)

  #WBIG = Array{Float64}(n, numTrials * k)
  #HBIG = Array{Float64}(m, numTrials * k)

  WBIG::Vector{Matrix} = []
  HBIG::Vector{Matrix} = []

  minObjValue = Inf
  bestW = Array{Float64}(n, k)
  bestH = Array{Float64}(k, m)
  residualMat = Array{Float64}(n, m)

  for idx in 1:numTrials
    WFile = WFiles[idx]
    HFile = HFiles[idx]
    W = loadMat(WFile)'
    H = loadMat(HFile)'
    verbose && println("size(W)=",size(W))
    verbose && println("size(H)=",size(H))
    @assert (n, k) == size(W)
    @assert (k, m) == size(H)

    push!(WBIG, W)
    push!(HBIG, H)
    #insertIntoBig!(WBIG, W, idx, k)
    #insertIntoBig!(HBIG, H', idx, k)


    # calc fit
    A_mul_B!(residualMat, W, H)
    BLAS.axpy!(-1, X, residualMat)
    objVal = vecnorm(residualMat)
    if objVal < minObjValue
      verbose && println("Min Obj = ", objVal)
      minObjValue = objVal
      bestW = W
      bestH = H
    end
  end

  zeroVecFix!(WBIG, verbose=verbose)
  zeroVecFix!(HBIG, verbose=verbose)

  verbose && println("Getting Clusters")
  verbose && println("size(HBIG)", size(HBIG))
  verbose && println("size(WBIG)", size(WBIG))
  clusterAssignments, M = NMFk.clusterSolutions((clusterWeights ? WBIG : HBIG), clusterWeights)
  if verbose
    println("ClusterAssignments:")
    show(IOContext(STDOUT, limit=true), "text/plain", clusterAssignments)
    println()
  end
  Wa, Ha, clusterSils = NMFk.finalize(WBIG, HBIG, clusterAssignments, clusterWeights, verbose=true)

  Wa = undoZeroFix(Wa, X)
  Ha = undoZeroFix(Ha, X)

  verbose && println("Mean Sil:", mean(clusterSils))
  verbose && println("Min Sil:", minimum(clusterSils))
  verbose && println("Max Sil:", maximum(clusterSils))


  # calc fit for A mats
  A_mul_B!(residualMat, Wa, Ha)
  BLAS.axpy!(-1, X, residualMat)
  avgNorm = vecnorm(residualMat)

  if (avgNorm < minObjValue)
    bestW = Wa
    bestH = Ha
  end

  println("avg / best run:", avgNorm / minObjValue)

  numParam = n*k + k*m
  numObs = n*m
  aic = 2 * numParam + numObs * log(minObjValue / numObs)
  println("Fit:", minObjValue, "\tSil:", minimum(clusterSils), "\t AIC:", aic)

  verbose && println("Writing ", outWPath)
  saveMat(outWPath, bestW)

  verbose && println("Writing ", outHPath)
  saveMat(outHPath, bestH)

  keyArr = []
  open(labelPath) do labelFile
    keyArr = [strip(line) for line in eachline(labelFile)]
  end

  clusterRes = Clustering.kmeans(bestH, k)
  clusters = hcat(keyArr, clusterRes.assignments)

  open(clusterDataPath, "w") do file
    serialize(file, (clusters, bestW, bestH))
  end

end

main()
