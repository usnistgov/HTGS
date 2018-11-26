//
// Created by tjb3 on 11/26/18.
//

#ifndef HTGS_PROFILEUTILS_HPP
#define HTGS_PROFILEUTILS_HPP


#include <string>

namespace htgs {

  class ProfileUtils {
   public:

    ProfileUtils(double totalTime) : totalTime(totalTime){ }

    /**
     * Gets the color for a given time relative to the entire graph's execution time.
     * @param time the time of the task
     * @return the hex color
     */
    std::string getColorForTime(double time) {
      int red = 0;
      int green = 0;
      int blue = 0;

      // compute percentage of totalTime
      int perc = (int) (time / totalTime * 100.0);

      if (perc % 10 != 0)
        perc = perc + 10 - (perc % 10);

      int index = (perc / 10);

      if (index < 0)
        index = 0;

      if (index >= 10)
        index = 9;

      red = rColor[index];
      green = gColor[index];
      blue = bColor[index];

      char hexcol[16];

      snprintf(hexcol, sizeof(hexcol), "%02x%02x%02x", red & 0xff, green & 0xff, blue & 0xff);
      std::string color(hexcol);
      color = "#" + color;

      return color;

    }

   private:
    int rColor[10] = {0, 0, 0, 0, 85, 170, 255, 255, 255, 255};
    int gColor[10] = {0, 85, 170, 255, 255, 255, 255, 170, 85, 0};
    int bColor[10] = {255, 255, 255, 255, 170, 85, 0, 0, 0, 0};

    double totalTime;

  };


}


#endif //HTGS_PROFILEUTILS_HPP
