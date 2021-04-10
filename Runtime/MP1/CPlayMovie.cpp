#include "Runtime/MP1/CPlayMovie.hpp"

namespace metaforce::MP1 {

const char* kMovies[] = {"Video/wingame.thp",       "Video/wingame_best.thp",  "Video/wingame_best.thp",
                         "Video/losegame.thp",      "Video/05_tallonText.thp", "Video/AfterCredits.thp",
                         "Video/SpecialEnding.thp", "Video/creditBG.thp"};

bool CPlayMovie::IsResultsScreen(EWhichMovie which) { return int(which) <= 2; }

CPlayMovie::CPlayMovie(EWhichMovie which) : CPlayMovieBase("CPlayMovie", kMovies[int(which)]), x18_which(which) {
  (void)x18_which;
}

} // namespace metaforce::MP1
