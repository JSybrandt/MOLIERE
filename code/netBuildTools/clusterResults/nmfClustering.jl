#!/home/jsybran/bin/julia-v0.6/bin/julia
#=
This code will cluster keywords within topics, and topics from the context of keywords.
=#


using ArgParse
import NMFk

type Topic
  keywords::Array{Tuple{String, Int}}
  frequency::Int
  Topic(f::Int) = new([], f)
end

function main()
  s = ArgParseSettings()
  @add_arg_table s begin
    "--topic-file", "-t"
      help = "Path to the topic model file. Note: this is after view_model.py is run."
      arg_type = String
      required = true
    "--keyword-vector-file", "-k"
      help = "Path to the keyword vector file. Note: this is the plaintext .vec file."
      arg_type = String
      required = true
    "--vector-size", "-s"
      help = "Dimensionality of the ngram vector space."
      arg_type = Int
      default = 500
    "--cluster-topics", "-C"
      help = "If set, cluster topics with respect to keywords."
      action = :store_true
    "--cluster-keywords", "-c"
      help = "If set, cluster keywords with respect to topics."
      action = :store_true
    "--verbose", "-v"
      action = :store_true
  end

  args = parse_args(ARGS, s)

  if (!args["cluster-topics"] && !args["cluster-keywords"]) ||
     (args["cluster-topics"] && args["cluster-keywords"])
    error("Must set either --cluster-topics or --cluster-keywords")
    exit(1)
  end

  if args["verbose"]
    println("Parsed:")
    for (k, v) in args
      println("$k ==> $v")
    end
  end

  topicFilePath = args["topic-file"]
  keywordVecFilePath = args["keyword-vector-file"]
  clusterTopics = args["cluster-topics"]
  clusterKeywords = args["cluster-keywords"]
  verbose = args["verbose"]
  intendedVecLength = args["vector-size"]

  topics::Array{Topic} = []
  keywords = Set{String}()

  verbose && println("Loading topics")

  open(topicFilePath) do topicFile
    for line in eachline(topicFile)
      tokens = split(lowercase(strip(line)))
      if "topic:" in tokens
        push!(topics, Topic(Int(parse(Float32, tokens[2]))))
        # verbose && println("Found topic $(length(topics))")
      elseif length(tokens) == 2
        push!(topics[end].keywords, (tokens[1], Int(parse(Float32, tokens[2]))))
        push!(keywords, tokens[1])
        # verbose && println("Found keyword / count $(tokens)")
      end
    end
  end

  if verbose
    println("Found $(length(topics)) topics")
    #=
    for i in 1:length(topics)
      println("Topic $(i-1) contains $(length(topics[i].keywords)) keywords.")
    end
    =#
    println("Found $(length(keywords)) keywords in total.")
  end

  if clusterTopics
    RunTopicClustering(keywordVecFilePath, topics)
  elseif clusterKeywords
    RunKeywordClustering(topics)
  end

end

function RunKeywordClustering(topics::Array{Topic})

end

function RunTopicClustering(vecFile::String, topics::Array{Topic})

  verbose && println("Loading vectors")
  word2vec = Dict{String, Vector{Float32}}()
  open(keywordVecFilePath) do vecFile
    for line in eachline(vecFile)
      tokens = split(strip(line))
      # skip the possible first line
      if length(tokens) > 2
        word = tokens[1]
        if word in keywords
          vec = map(x->parse(Float32, x), tokens[2:end])
          if length(vec) != intendedVecLength
            error("Vector for $word is of length $(length(vec)) != $intendedVecLength")
            exit(1)
          end
          word2vec[word] = vec
        end
      end
    end
  end

  verbose && println("Getting centroids")

  function getCentroid(topic::Topic)
    sum = zeros(Float32, intendedVecLength)
    count = 0
    for (word, freq) in topic.keywords
      if haskey(word2vec, word)
        sum += word2vec[word] * freq
        count += freq
      end
    end
    return sum / count
  end

  function cosSim(A::Vector, B::Vector)
    dot = 0
    for i in 1:length(A)
      dot += abs(A[i]*B[i])
    end
    return convert(Float32, dot / (norm(A) * norm(B)))
  end

  TopicMatrix = zeros(length(topics), length(topics))
  centroids = map(x->getCentroid(x), topics)
  for i in 1:length(topics)
    for j in i:length(topics)
      sim = cosSim(centroids[i],centroids[j])
      TopicMatrix[i,j] = sim
      TopicMatrix[j,i] = sim
    end
  end

  RunNMFk(TopicMatrix)
end

function RunNMFk(A :: Matrix)
  trailKs = 2:5
  res = NMFk.execute(A, trailKs)

  # res is a 5 dim array with:
  # Ws, Hs, Fit, Sil, AIC
  largestDrop = 0
  bestK = 0
  for idx in 2:4
    drop = abs(res[4][idx] - res[4][idx+1])
    verbose && println("Drop from $idx to $(idx+1) is $drop")
    if largestDrop < drop
      largestDrop = drop
      bestK = idx
    end
  end

  verbose && println("Determined K=$bestK")
  @show res[1][bestK]
  @show res[2][bestK]

  clusters = zeros(size(res[2][bestK]))
  # this is gross code which means
  # we are putting a 1 in clusters[i][j]
  # if topic j is in cluster i
  clusters[findmax(res[2][bestK], 1)[2]] = 1
  for topic in 1:size(clusters, 2)
    clusterId = findmax(clusters[:, topic])[2]
    verbose && println("Topic: $(topic-1)\tCluter: $clusterId")
  end
end

main()
