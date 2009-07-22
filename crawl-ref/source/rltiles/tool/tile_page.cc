#include "tile_page.h"
#include "tile_colour.h"
#include <stdio.h>
#include <string.h>
#include "tile.h"

tile_page::tile_page() : m_width(1024), m_height(0)
{
}

tile_page::~tile_page()
{
    for (unsigned int i = 0; i < m_tiles.size(); i++)
        delete m_tiles[i];

    m_tiles.clear();
    m_counts.clear();
}

bool tile_page::place_images()
{
    // locate all the tiles on the page, so we can determine its size
    // and the tex coords.

    m_offsets.clear();
    m_texcoords.clear();

    unsigned int ymin, ycur, ymax;
    unsigned int xmin, xcur, xmax;
    ymin = ycur = ymax = xmin = xcur = xmax = 0;

    for (unsigned int i = 0; i < m_tiles.size(); i++)
    {
        unsigned int ofs_x, ofs_y, tilew, tileh;
        if (m_tiles[i]->shrink())
            m_tiles[i]->get_bounding_box(ofs_x, ofs_y, tilew, tileh);
        else
        {
            ofs_x = 0;
            ofs_y = 0;
            tilew = m_tiles[i]->width();
            tileh = m_tiles[i]->height();
        }

        m_offsets.push_back(ofs_x);
        m_offsets.push_back(ofs_y);
        m_offsets.push_back(m_tiles[i]->width());
        m_offsets.push_back(m_tiles[i]->height());

        if (xcur + tilew > m_width)
        {
            ycur = ymin = ymax;
            xcur = xmin = xmax = 0;
        }

        if (tileh + ycur >= ymax)
        {
            if (ycur != ymin)
            {
                ycur = ymin;
                xcur = xmax;
                xmin = xmax = xcur;
            }

            if (xcur + tilew > m_width)
            {
                ycur = ymin = ymax;
                xcur = xmin = xmax = 0;
            }

            if (ycur == ymin)
                ymax = std::max(ymin + (int)tileh, ymax);
        }

        m_height = ymax;

        m_texcoords.push_back(xcur);
        m_texcoords.push_back(ycur);
        m_texcoords.push_back(xcur + tilew);
        m_texcoords.push_back(ycur + tileh);

        // Only add downwards, stretching out xmax as we go.
        xmax = std::max(xmax, xcur + (int)tilew);
        xcur = xmin;
        ycur += tileh;
    }

    return (true);
}

bool tile_page::write_image(const char *filename)
{
    if (m_width * m_height <= 0)
    {
        fprintf(stderr, "Error: failed to write image.  No images placed?\n");
        return (false);
    }

    tile_colour *pixels = new tile_colour[m_width * m_height];
    memset(pixels, 0, m_width * m_height * sizeof(tile_colour));

    for (unsigned int i = 0; i < m_tiles.size(); i++)
    {
        int sx = m_texcoords[i*4];
        int sy = m_texcoords[i*4+1];
        int ex = m_texcoords[i*4+2];
        int ey = m_texcoords[i*4+3];
        int wx = ex - sx;
        int wy = ey - sy;
        int ofs_x = m_offsets[i*4];
        int ofs_y = m_offsets[i*4+1];

        for (int y = 0; y < wy; y++)
            for (int x = 0; x < wx; x++)
            {
                tile_colour &dest = pixels[(sx+x) + (sy+y)*m_width];
                tile_colour &src = m_tiles[i]->get_pixel(ofs_x+x, ofs_y+y);
                dest = src;
            }
    }

    bool success = write_png(filename, pixels, m_width, m_height);
    delete[] pixels;
    return success;
}
