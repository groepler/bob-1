#include <QPainter>

#include "visioner/cv/cv_draw.h"

namespace bob { namespace visioner {

  QImage draw_gt(const ipscale_t& ipscale)
  {
    QImage qimage = visioner::convert(ipscale.m_image);

    QPainter painter(&qimage);
    painter.setPen(QPen(QBrush(qRgb(0, 255, 0)), 2.0, Qt::SolidLine));

    // Draw each object
    for (objects_t::const_iterator ito = ipscale.m_objects.begin();
        ito != ipscale.m_objects.end(); ++ ito)
    {
      // Bounding box
      const Object& object = *ito;
      painter.drawRect(object.bbx());

      // Keypoints
      for (keypoints_t::const_iterator itk = object.keypoints().begin();
          itk != object.keypoints().end(); ++ itk)
      {
        const visioner::Keypoint& keypoint = *itk;
        const visioner::point_t& point = keypoint.m_point;

        painter.drawLine(point.x() - 4, point.y(), point.x() + 4, point.y());
        painter.drawLine(point.x(), point.y() - 4, point.x(), point.y() + 4);
      }
    }

    return qimage;
  }

  void draw_detection(QImage& qimage, const detection_t& detection, const param_t& param, bool label)
  {
    QPainter painter(&qimage);

    QFont font = painter.font();
    font.setBold(true);
    painter.setFont(font);
    static const QPen pen_true(QBrush(qRgb(0, 0, 255)), 2.0, Qt::SolidLine);
    static const QPen pen_false(QBrush(qRgb(255, 0, 0)), 2.0, Qt::SolidLine);

    const scalar_t& score = detection.first;
    const rect_t& bbx = detection.second.first;                
    const int output = detection.second.second;

    // Detection: bounding box
    painter.setPen(label == true ? pen_true : pen_false);
    painter.drawRect(bbx);

    // Detection: score (3D)
    const QString text_score = QObject::tr("%1").arg(score, 0, 'f', 2);

    painter.setPen(qRgb(155, 155, 255));
    painter.drawText(bbx.left() + 1,
        bbx.top() + 1 + painter.fontMetrics().height(),
        text_score);

    painter.setPen(qRgb(105, 105, 255));
    painter.drawText(bbx.left() + 2,
        bbx.top() + 2 + painter.fontMetrics().height(),
        text_score);

    painter.setPen(qRgb(55, 55, 255));
    painter.drawText(bbx.left() + 3,
        bbx.top() + 3 + painter.fontMetrics().height(),
        text_score);

    // Detection: label
    painter.setPen(label == true ? pen_true : pen_false);
    const QString text_label = QObject::tr("%1").arg(param.m_labels[output].c_str());
    painter.drawText(bbx.right() - painter.fontMetrics().width(text_label) - 2, 
        bbx.bottom() - 2,
        text_label);                
  }

  void draw_detections(QImage& qimage, const detections_t& detections, const param_t& param, const bools_t& labels)
  {
    if (detections.size() == labels.size())
    {
      for (index_t i = 0; i < detections.size(); i ++)
      {
        draw_detection(qimage, detections[i], param, labels[i]);
      }
    }
  }

  void draw_points(QImage& qimage, const points_t& points)
  {
    QPainter painter(&qimage);

    painter.setPen(QPen(QBrush(qRgb(255, 0, 0)), 2.0, Qt::SolidLine));
    for (points_t::const_iterator it = points.begin(); it != points.end(); ++ it)
    {
      const point_t& point = *it;

      painter.drawLine(point.x() - 4, point.y(), point.x() + 4, point.y());
      painter.drawLine(point.x(), point.y() - 4, point.x(), point.y() + 4);
    }
  }

  void draw_label(QImage& qimage, const detection_t& detection, const param_t& param, 
      index_t gt_label, index_t dt_label)
  {
    QPainter painter(&qimage);

    QFont font = painter.font();
    font.setBold(true);
    painter.setFont(font);

    static const QPen pen_true(QBrush(qRgb(0, 0, 255)), 2.0, Qt::SolidLine);
    static const QPen pen_false(QBrush(qRgb(255, 0, 0)), 2.0, Qt::SolidLine);

    const rect_t& bbx = detection.second.first;                

    painter.setPen(gt_label == dt_label ? pen_true : pen_false);
    const QString text_label = QObject::tr("<%1> / <%2>")
      .arg(param.m_labels[dt_label].c_str())
      .arg(param.m_labels[gt_label].c_str());
    painter.drawText(bbx.left() + 2, 
        bbx.top() - 4 * painter.fontMetrics().height() / 5,
        text_label);    
  }

}}
