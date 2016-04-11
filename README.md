# OldGameTheoryProjects
Found this forgotten C++ code I wrote ages ago, uploaded here so I don't lose it again

It's been long enough that I don't remember all the details but the overall idea was to come up with a _very_ approximate solution to 2-player poker (Texas Hold'em specifically) with the following strategy:
* Decompose all the different combinations of hands and boards into a tractable set of common situations ("top pair no draw on a board with a flush draw")
* Map the above situations to a multi-step (step = betting action or street) game where both players have a score of \[0,1\] (ie hand strength) _but also_ a probability distribution function that describes how their hand will vary with time (ie flush draw will stay at ~0 80% of the time, move to ~1 20% of the time)
* Use a variant of [fictious play (FP)](https://en.wikipedia.org/wiki/Fictitious_play) on this simplified version (for >1 step it's still right at the edge of what FP can calculate in a sensible amount of time as I recall)

There are 2 projects here:
* BoardAnalyzer is a mostly complete and functioning library that maps specific situations (and generates probabilities for them)
   * There are some examples included that use the library to solve some very simple simplified games (eg your opponent bets on the flop X% of the time, if your only two options are to fold or raise all-in, what should your range be?)
   * Most of the fun is in [CBoardAnalyzer](https://github.com/Alex-At-Home/OldGameTheoryProjects/blob/master/BoardAnalyzer/CBoardAnalyzer.cpp).
* ZeroOneGames is some pretty complicated code to generate approximate solutions to the "\[0, 1\]" multi-step game described above, its details are a bit hazy, but it looks like:
   * [ZeroOneGames_altConv](https://github.com/Alex-At-Home/OldGameTheoryProjects/blob/master/ZeroOneGames/ZeroOneGames_altConv.cpp) handled the FP
   * [ZeroOneGames_Threshold](https://github.com/Alex-At-Home/OldGameTheoryProjects/blob/master/ZeroOneGames/ZeroOneGames_Threshold.cpp) handled the multi-step and board change logic
