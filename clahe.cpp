#include "clahe.hpp"
#include "common_code.hpp"
#include <opencv2/imgproc.hpp>

static uchar interpolate_corner(const cv::Mat &image,
                                const std::vector<cv::Mat> &tables,
                                const cv::Point &point,
                                const cv::Size &cell_size,
                                const cv::Size &grid_size)
{
    const int offset_x = (cell_size.width + 1) >> 1;
    const int offset_y = (cell_size.height + 1) >> 1;

    CV_Assert((point.x < offset_x && point.y < offset_y) ||
              (point.x < offset_x && point.y >= (image.rows - offset_y - 1)) ||
              (point.x >= (image.cols - offset_x - 1) && point.y < offset_x) ||
              (point.x >= (image.cols - offset_x - 1) && point.y >= (image.rows - offset_y - 1)));

    const int row = point.y / cell_size.height;
    const int col = point.x / cell_size.width;
    const int index = row * grid_size.width + col;

    return tables[index].at<uchar>(image.at<uchar>(point));
}

static uchar linear_interpolate_rows(const cv::Mat &image,
                                     const std::vector<cv::Mat> &tables,
                                     const cv::Point &point,
                                     const cv::Size &cell_size,
                                     const cv::Size &grid_size)
{
    const int offset_x = cell_size.width >> 1;
    const int offset_y = cell_size.height >> 1;

    CV_Assert(point.x >= offset_x &&
              point.x < (image.cols - offset_x - 1) &&
              (point.y < offset_y ||
               point.y >= (image.rows - offset_y - 1)));

    const int row = (point.y - offset_y) / cell_size.height;
    const int left_col = (point.x - offset_x) / cell_size.width;
    const int right_col = left_col + 1;

    const float right_center = right_col * cell_size.width + offset_x;

    const float weight_left = (right_center - point.x) / cell_size.width;
    const float weight_right = 1.0 - weight_left;

    const int left_index = row * grid_size.width + left_col;
    const int right_index = row * grid_size.width + right_col;

    const uchar pixel = image.at<uchar>(point);

    uchar result = weight_left * tables[left_index].at<uchar>(pixel) +
                   weight_right * tables[right_index].at<uchar>(pixel);

    return result;
}

static uchar linear_interpolate_cols(const cv::Mat &image,
                                     const std::vector<cv::Mat> &tables,
                                     const cv::Point &point,
                                     const cv::Size &cell_size,
                                     const cv::Size &grid_size)
{
    const int offset_x = cell_size.width >> 1;
    const int offset_y = cell_size.height >> 1;

    CV_Assert(point.y >= offset_y &&
              point.y < (image.rows - offset_y - 1) &&
              (point.x < offset_x ||
               point.x >= (image.cols - offset_x - 1)));

    const int col = point.x / cell_size.width;
    const int upper_row = (point.y - offset_y) / cell_size.height;
    const int lower_row = upper_row + 1;

    const float lower_center = lower_row * cell_size.height + offset_y;

    const float weight_upper = (lower_center - point.y) / cell_size.height;
    const float weight_lower = 1.0 - weight_upper;

    const int upper_index = upper_row * grid_size.width + col;
    const int lower_index = lower_row * grid_size.width + col;

    const uchar pixel = image.at<uchar>(point);

    uchar result = weight_upper * tables[upper_index].at<uchar>(pixel) +
                   weight_lower * tables[lower_index].at<uchar>(pixel);

    return result;
}

static uchar bilinear_interpolate(const cv::Mat &image,
                                  const std::vector<cv::Mat> &tables,
                                  const cv::Point &point,
                                  const cv::Size &cell_size,
                                  const cv::Size &grid_size)
{
    const int offset_x = cell_size.width >> 1;
    const int offset_y = cell_size.height >> 1;

    CV_Assert(point.y >= offset_y &&
              point.y < (image.rows - offset_y - 1) &&
              point.x >= offset_x &&
              point.x < (image.cols - offset_x - 1));

    const int col1 = (point.x - offset_x) / cell_size.width;
    const int row1 = (point.y - offset_y) / cell_size.height;

    const int col2 = col1 + 1;
    const int row2 = row1;

    const int col3 = col1 + 1;
    const int row3 = row1 + 1;

    const int col4 = col1;
    const int row4 = row1 + 1;

    const float center_x = col3 * cell_size.width + offset_x;
    const float center_y = row3 * cell_size.height + offset_y;

    const int index11 = row1 * grid_size.width + col1;
    const int index12 = row2 * grid_size.width + col2;
    const int index22 = row3 * grid_size.width + col3;
    const int index21 = row4 * grid_size.width + col4;

    const uchar pixel = image.at<uchar>(point);

    const uchar value11 = tables[index11].at<uchar>(pixel);
    const uchar value12 = tables[index12].at<uchar>(pixel);
    const uchar value22 = tables[index22].at<uchar>(pixel);
    const uchar value21 = tables[index21].at<uchar>(pixel);

    const float weight_x = (center_x - point.x) / cell_size.width;

    const float upper_value =
        weight_x * value11 + (1.0f - weight_x) * value12;

    const float lower_value =
        weight_x * value21 + (1.0f - weight_x) * value22;

    const float weight_y = (center_y - point.y) / cell_size.height;

    const uchar result = cv::saturate_cast<uchar>(
        weight_y * upper_value + (1.0f - weight_y) * lower_value);

    return result;
}

static uchar compute_interpolate_value(const cv::Mat &image,
                                       const std::vector<cv::Mat> &tables,
                                       const cv::Point &point,
                                       const cv::Size &cell_size,
                                       const cv::Size &grid_size)
{
    uchar result = image.at<uchar>(point);

    const int offset_x = cell_size.width >> 1;
    const int offset_y = cell_size.height >> 1;

    const bool inside_rows =
        point.y >= offset_y &&
        point.y < (image.rows - offset_y - 1);

    const bool inside_cols =
        point.x >= offset_x &&
        point.x < (image.cols - offset_x - 1);

    if (inside_rows && inside_cols)
    {
        result = bilinear_interpolate(
            image, tables, point, cell_size, grid_size);
    }
    else if (inside_rows && !inside_cols)
    {
        result = linear_interpolate_cols(
            image, tables, point, cell_size, grid_size);
    }
    else if (inside_cols && !inside_rows)
    {
        result = linear_interpolate_rows(
            image, tables, point, cell_size, grid_size);
    }
    else
    {
        result = interpolate_corner(
            image, tables, point, cell_size, grid_size);
    }

    return result;
}

cv::Mat fsiv_clahe(const cv::Mat &input, float s, int radius)
{
    CV_Assert(input.type() == CV_8UC1);

    cv::Mat output = input.clone();

    if (radius == 0)
    {
        // Equalization using the histogram of the complete image.
        cv::Mat histogram = fsiv_compute_image_histogram(input);

        cv::Mat table =
            fsiv_create_equalization_lookup_table(histogram, s);

        fsiv_apply_lookup_table(input, table, output);
    }
    else
    {
        // Divide the image into cells of the specified size.
        const cv::Size cell_size(2 * radius + 1, 2 * radius + 1);

        cv::Mat extended = input;

        // Extend the image when its dimensions are not multiples
        // of the cell dimensions.
        if ((input.rows % cell_size.height) != 0 ||
            (input.cols % cell_size.width) != 0)
        {
            const int extra_rows =
                cell_size.height - (input.rows % cell_size.height);

            const int extra_cols =
                cell_size.width - (input.cols % cell_size.width);

            extended = cv::Mat(
                input.rows + extra_rows,
                input.cols + extra_cols,
                input.type());

            cv::copyMakeBorder(
                input, extended,
                0, extra_rows,
                0, extra_cols,
                cv::BORDER_REFLECT101);
        }

        const cv::Size grid_size(
            extended.cols / cell_size.width,
            extended.rows / cell_size.height);

        // Generate an equalization table for every cell.
        std::vector<cv::Mat> tables(grid_size.area());

        for (int row = 0; row < grid_size.height; ++row)
        {
            for (int col = 0; col < grid_size.width; ++col)
            {
                const int index = row * grid_size.width + col;

                const cv::Rect region(
                    col * cell_size.width,
                    row * cell_size.height,
                    cell_size.width,
                    cell_size.height);

                cv::Mat histogram =
                    fsiv_compute_image_histogram(extended(region));

                tables[index] =
                    fsiv_create_equalization_lookup_table(histogram, s);
            }
        }

        // Equalize the original pixels using the neighboring tables.
        for (int row = 0; row < input.rows; ++row)
        {
            for (int col = 0; col < input.cols; ++col)
            {
                output.at<uchar>(row, col) =
                    compute_interpolate_value(
                        extended,
                        tables,
                        cv::Point(col, row),
                        cell_size,
                        grid_size);
            }
        }
    }

    CV_Assert(output.size() == input.size());
    CV_Assert(output.type() == input.type());

    return output;
}