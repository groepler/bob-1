#ifndef BOB_VISIONER_GEOM_H
#define BOB_VISIONER_GEOM_H

#include <QRectF>
#include <QPointF>
#include <vector>
#include <algorithm>
#include <numeric>

namespace bob { namespace visioner {

  typedef QPointF				point_t;
  typedef std::vector<point_t>		points_t;
  typedef points_t::const_iterator	points_const_it;
  typedef points_t::iterator		points_it;

  typedef QRectF				rect_t;
  typedef std::vector<rect_t>		rects_t;
  typedef rects_t::const_iterator		rects_const_it;
  typedef rects_t::iterator		rects_it;

  // Sub-window (location + scale)
  struct sw_t
  {
    sw_t(int x = 0, int y = 0, int s = 0)
      :	m_x(x), m_y(y), m_s(s)
    {				
    }

    u_int16_t       m_x, m_y;
    u_int32_t       m_s;
  };
  typedef std::vector<sw_t>	sws_t;

  // Region of interest
  struct roi_t
  {
    roi_t(int min_x, int max_x, int min_y, int max_y)
      :	m_min_x(min_x), m_max_x(max_x),
      m_min_y(min_y), m_max_y(max_y)
    {			
    }

    int16_t		m_min_x, m_max_x;
    int16_t		m_min_y, m_max_y;
  };
  typedef std::vector<roi_t>	rois_t;

  // Area of a rectangle	
  inline qreal area(const rect_t& rect)
  {
    return rect.width() * rect.height();
  }	

  // Compare two sub-windows
  inline bool operator<(const sw_t& one, const sw_t& two)
  {
    return one.m_s < two.m_s ||
      (one.m_s == two.m_s && one.m_x < two.m_x) ||
      (one.m_x == two.m_x && one.m_y < two.m_y);
  }

}}

// Compare two rectangles
inline bool operator<(const bob::visioner::rect_t& one, const bob::visioner::rect_t& two)
{
  return  one.top() < two.top() || (one.top() == two.top() && 
      (one.left() < two.left() || (one.left() == two.left() && 
                                   (one.width() < two.width() || (one.width() == two.width() && 
                                                                  one.height() < two.height())))));
}

#endif // BOB_VISIONER_GEOM_H
