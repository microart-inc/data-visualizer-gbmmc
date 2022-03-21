#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Box.H>
#include <vector>
#include <fstream>
#include <iostream>
#include <cmath>
#include <chrono>

bool darkMode = false;
int width = 1280;
int height = 720;
uint *colors;
std::vector<int> weightedDeviations;
Fl_Output *status;
Fl_Float_Input *permInput;
Fl_Float_Input *daysInput;
std::vector<float> closes;

int randomDeviation()
{
    int randNum = rand() % (weightedDeviations.size() - 0 + 1) + 0;
    return weightedDeviations[randNum];
}

float forecastStockPrice(float currentPrice, float mean, float deviation)
{
    float sway = mean - pow(deviation, 2) / 2;
    float monteCarloDeviation = deviation * randomDeviation();
    return currentPrice * exp(sway + monteCarloDeviation);
}

std::vector<std::vector<float>> compute(std::vector<float> data, int permutations, int days)
{
    std::vector<float> stockPercentChangeByDay;

    float sum = 0;

    for (int i = 1; i < data.size(); i++)
    {
        float currentClosePrice = data[i];
        float previousClosePrice = data[i - 1];
        stockPercentChangeByDay.push_back((currentClosePrice - previousClosePrice) / previousClosePrice);
        sum += stockPercentChangeByDay[i - 1];
    }

    float mean = sum / stockPercentChangeByDay.size();
    float deviation_temp = 0;
    for (float &item : stockPercentChangeByDay)
    {
        deviation_temp += (item - mean) * (item - mean);
    }
    float deviation = sqrt(deviation_temp / stockPercentChangeByDay.size());

    std::vector<std::vector<float>> forecast;

    for (int i = 0; i < permutations; i++)
    {
        forecast.push_back(std::vector<float>());
        for (int j = 0; j < days; j++)
        {
            float currentPrice = j == 0 ? data[data.size() - 1] : forecast[i][j - 1];
            forecast[i].push_back(forecastStockPrice(currentPrice, mean, deviation));
        }
    }
    return forecast;
}

uint getRandomColor()
{
    int index = rand() % 6;
    return colors[index];
}

class MultiLine : public Fl_Widget
{
public:
    int _x;
    int _y;
    int _w;
    int _h;
    std::vector<std::vector<float>> data = {};

    MultiLine(int x, int y, int w, int h) : Fl_Widget(x, y, w, h)
    {
        _x = x;
        _y = y;
        _w = w;
        _h = h;
    }

    void draw()
    {
        if (data.size() <= 0)
            return;
        /*
        fl_color(darkMode ? FL_BLACK : FL_WHITE);
        fl_rectf(_x, _y, _w, _h);
        fl_color(darkMode ? FL_WHITE : FL_BLACK);
        fl_line_style(FL_SOLID, 1);
        fl_line(_x, _y, _x + _w, _y + _h);
        fl_line(_x, _y + _h, _x + _w, _y);
        */
        fl_color(darkMode ? FL_BLACK : FL_WHITE);
        fl_rectf(0, 0, width, height);
        float dataPointWidth = width / (data[0].size() - 40);
        float dataPointHeight = 1;
        fl_color(darkMode ? FL_WHITE : FL_BLACK);
        fl_line_style(FL_SOLID, 1);
        for (int i = 0; i < data.size(); i++)
        {
            fl_color(getRandomColor());
            for (int j = 0; j < data[i].size(); j++)
            {
                if (j == 0)
                    continue;

                float h1 = height - (data[i][j - 1] * dataPointHeight);
                float h2 = height - (dataPointHeight * data[i][j]);
                if (h1 <= 0 || h1 > height)
                    h1 = height - data[i][j - 2];
                if (h2 <= 0 || h2 > height)
                    h2 = height - data[i][j - 2];
                fl_line((j - 1) * dataPointWidth, h1, dataPointWidth * j, h2);
            }
        }
    }

    void setData(std::vector<std::vector<float>> data)
    {
        this->data = data;
    }
};

std::vector<float> readFileData()
{
    std::ifstream file("closes.json");
    std::vector<float> closes;
    char c;
    std::string number;
    while (file >> c)
    {
        if (c == '[')
        {
            // Start of JSON
            number = "";
        }
        else if (c == ',')
        {
            // End of number
            closes.push_back(std::stof(number));
            number = "";
        }
        else if (c == ']')
        {
            // End of JSON
            closes.push_back(std::stof(number));
            break;
        }
        else
        {
            number += c;
        }
    }
    file.close();
    return closes;
}

void onComputeRequested(Fl_Widget *widget, void *boxPtr)
{
    float permutations = std::stof(permInput->value());
    float days = std::stof(daysInput->value());
    auto startTime = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<float>> data = compute(closes, permutations, days);
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    // Get multiline from pointer
    MultiLine *box = (MultiLine *)boxPtr;
    if (data.size() <= 0)
    {
        status->textcolor(FL_RED);
        status->value("Can't compute 0 permutations");
        return;
    }
    status->textcolor(FL_GREEN);
    float dur = (float)duration.count() / 1000;
    std::__cxx11::basic_string<char> text = "Computed " + std::to_string(data.size()) + " permutations in " + std::to_string(dur) + "ms";
    status->value(text.c_str());
    box->setData(data);
    box->redraw();
}

int main(int argc, char **argv)
{
    closes = readFileData();
    darkMode = true;
    width = 1280;
    height = 720;
    colors = new uint[5]{FL_RED, FL_GREEN, FL_YELLOW, FL_MAGENTA, FL_CYAN};
    for (int i = 0; i < 2000; i++)
    {
        if (i > 1682)
        {
            weightedDeviations.push_back(2);
        }
        else if (i > 1003)
        {
            weightedDeviations.push_back(1);
        }
        else if (i > 1000)
        {
            weightedDeviations.push_back(3);
        }
        else if (i > 682)
        {
            weightedDeviations.push_back(-2);
        }
        else if (i > 3)
        {
            weightedDeviations.push_back(-1);
        }
        else
        {
            weightedDeviations.push_back(-3);
        }
    }
    Fl_Window *window = new Fl_Window(width, height, "Data Visualizer | Muddassir | IB Math IA");
    window->color(darkMode ? FL_BLACK : FL_WHITE);
    fl_color(darkMode ? FL_WHITE : FL_BLACK);

    MultiLine *box = new MultiLine(0, 40, width, height - 40);

    permInput = new Fl_Float_Input(120, 5, 100, 30, "Permutations: ");
    permInput->labelcolor(darkMode ? FL_WHITE : FL_BLACK);
    permInput->value("0");

    daysInput = new Fl_Float_Input((2 * permInput->x()) + permInput->w() - 30, 5, 100, 30, "Days: ");
    daysInput->labelcolor(darkMode ? FL_WHITE : FL_BLACK);
    daysInput->value("0");

    Fl_Button *compute = new Fl_Button(((3 * permInput->x()) + permInput->w()), 5, 100, 30, "Compute");
    compute->color(darkMode ? FL_WHITE : FL_BLACK);
    compute->labelcolor(darkMode ? FL_BLACK : FL_WHITE);
    compute->callback(onComputeRequested, box);

    status = new Fl_Output((4 * permInput->x()) + permInput->w(), 5, 600, 30);
    status->textcolor(FL_GREEN);
    status->color(darkMode ? FL_BLACK : FL_WHITE);
    status->color2(darkMode ? FL_BLACK : FL_WHITE);
    status->value("Ready");
    status->box(FL_FLAT_BOX);

    window->end();
    window->show(argc, argv);
    return Fl::run();
}