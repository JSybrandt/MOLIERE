#pragma once

#include<unordered_set>
#include<string>

const std::unordered_set<std::string> STOP_WORDS {
  ".",
  "1st",
  "2nd",
  "3rd",
  "about",
  "above",
  "abs",
  "accordingly",
  "across",
  "after",
  "afterwards",
  "again",
  "against",
  "all",
  "almost",
  "alone",
  "along",
  "already",
  "also",
  "although",
  "always",
  "am",
  "among",
  "amongst",
  "an",
  "analyze",
  "and",
  "another",
  "any",
  "anyhow",
  "anyone",
  "anything",
  "anywhere",
  "applicable",
  "apply",
  "are",
  "arise",
  "around",
  "as",
  "assume",
  "at",
  "be",
  "became",
  "because",
  "become",
  "becomes",
  "becoming",
  "been",
  "before",
  "beforehand",
  "being",
  "below",
  "beside",
  "besides",
  "between",
  "beyond",
  "both",
  "but",
  "by",
  "came",
  "can",
  "cannot",
  "cc",
  "cm",
  "come",
  "compare",
  "compare",
  "could",
  "de",
  "dealing",
  "department",
  "depend",
  "did",
  "discover",
  "dl",
  "do",
  "does",
  "done",
  "due",
  "during",
  "each",
  "ec",
  "ed",
  "effected",
  "effective",
  "eg",
  "either",
  "else",
  "elsewhere",
  "enough",
  "especially",
  "et",
  "etc",
  "ever",
  "every",
  "everyone",
  "everything",
  "everywhere",
  "except",
  "find",
  "finding",
  "findings",
  "found",
  "for",
  "found",
  "from",
  "further",
  "gave",
  "get",
  "give",
  "go",
  "gone",
  "got",
  "gov",
  "had",
  "has",
  "have",
  "having",
  "he",
  "hence",
  "her",
  "here",
  "hereafter",
  "hereby",
  "herein",
  "hereupon",
  "hers",
  "herself",
  "him",
  "himself",
  "his",
  "how",
  "however",
  "hr",
  "ie",
  "if",
  "ii",
  "iii",
  "identify",
  "identified",
  "immediately",
  "importance",
  "important",
  "in",
  "inc",
  "incl",
  "indeed",
  "into",
  "investigate",
  "investigated",
  "is",
  "it",
  "its",
  "itself",
  "just",
  "keep",
  "kept",
  "kg",
  "km",
  "last",
  "latter",
  "latterly",
  "lb",
  "ld",
  "letter",
  "like",
  "ltd",
  "made",
  "mainly",
  "major",
  "make",
  "many",
  "may",
  "me",
  "mean",
  "meanwhile",
  "mg",
  "might",
  "ml",
  "mm",
  "mo",
  "more",
  "moreover",
  "most",
  "mostly",
  "mr",
  "much",
  "mug",
  "must",
  "my",
  "myself",
  "myself",
  "namely",
  "nearly",
  "necessarily",
  "neither",
  "never",
  "nevertheless",
  "next",
  "no",
  "nobody",
  "noone",
  "nor",
  "normally",
  "nos",
  "not",
  "noted",
  "nothing",
  "novel",
  "now",
  "nowhere",
  "obtained",
  "of",
  "off",
  "often",
  "on",
  "only",
  "onto",
  "or",
  "other",
  "others",
  "otherwise",
  "ought",
  "our",
  "ours",
  "ourselves",
  "out",
  "over",
  "overall",
  "owing",
  "own",
  "oz",
  "particularly",
  "per",
  "perhaps",
  "pm",
  "precede",
  "predominantly",
  "present",
  "presently",
  "previously",
  "primarily",
  "promptly",
  "pt",
  "quickly",
  "quite",
  "quot",
  "rather",
  "readily",
  "really",
  "recently",
  "refs",
  "regarding",
  "relate",
  "said",
  "same",
  "seem",
  "seemed",
  "seeming",
  "seems",
  "seen",
  "seriously",
  "several",
  "shall",
  "she",
  "should",
  "show",
  "showed",
  "shown",
  "shown",
  "shows",
  "significant",
  "significantly",
  "since",
  "similar",
  "slightly",
  "so",
  "some",
  "somehow",
  "someone",
  "something",
  "sometime",
  "sometimes",
  "somewhat",
  "somewhere",
  "soon",
  "specifically",
  "still",
  "strongly",
  "studied",
  "sub",
  "suggest",
  "suggested",
  "substantially",
  "such",
  "sufficiently",
  "take",
  "tell",
  "th",
  "than",
  "that",
  "the",
  "their",
  "theirs",
  "them",
  "themselves",
  "then",
  "thence",
  "there",
  "thereafter",
  "thereby",
  "therefore",
  "therein",
  "thereupon",
  "these",
  "they",
  "this",
  "thorough",
  "those",
  "though",
  "through",
  "throughout",
  "thru",
  "thus",
  "to",
  "together",
  "too",
  "toward",
  "towards",
  "try",
  "type",
  "ug",
  "under",
  "unless",
  "until",
  "up",
  "upon",
  "us",
  "use",
  "used",
  "usefully",
  "usefulness",
  "using",
  "usually",
  "various",
  "very",
  "via",
  "was",
  "we",
  "were",
  "what",
  "whatever",
  "when",
  "whence",
  "whenever",
  "where",
  "whereafter",
  "whereas",
  "whereby",
  "wherein",
  "whereupon",
  "wherever",
  "whether",
  "which",
  "while",
  "whither",
  "who",
  "whoever",
  "whom",
  "whose",
  "why",
  "will",
  "with",
  "within",
  "without",
  "wk",
  "would",
  "wt",
  "yet",
  "you",
  "your",
  "yours",
  "yourself",
  "yourselves",
  "yr",
  "a",
  "b",
  "c",
  "d",
  "e",
  "f",
  "g",
  "h",
  "i",
  "j",
  "k",
  "l",
  "m",
  "n",
  "o",
  "p",
  "q",
  "r",
  "s",
  "t",
  "u",
  "v",
  "w",
  "x",
  "y",
  "z",
  "0",
  "1",
  "2",
  "3",
  "4",
  "5",
  "6",
  "7",
  "8",
  "9",
  "zero",
  "one",
  "two",
  "three",
  "four",
  "five",
  "six",
  "seven",
  "eight",
  "nine",
  "wa",
  "is_a",
  "=",
  "<",
  ">",
  "</s>",
  "associated_with",
  "associated",
  "effects_of"
};
