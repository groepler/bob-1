#ifndef BOB_VISIONER_MB_LBP_MODEL_H
#define BOB_VISIONER_MB_LBP_MODEL_H

#include "visioner/model/models/ii_model.h"
#include "visioner/vision/mb_xlbp.h"
#include "visioner/vision/mb_xmct.h"

namespace bob { namespace visioner {

  /////////////////////////////////////////////////////////////////////////////////////////
  // Multi-block dense feature pool of (t/d/m)LBP/MCT codes.
  //      All codes are used.
  /////////////////////////////////////////////////////////////////////////////////////////

  static const string_t LBPNames[] = { "LBP",
    "mLBP", 
    "tLBP", 
    "dLBP",
    "MCT"};

  template <index_t (*TLBPOp) (const igreyimage_t&, int, int, int, int), int TNameIndex, int NFeatureValues> class MBxxxLBPModel : public IIModel {

    public:

      // Constructor
      MBxxxLBPModel(const param_t& param = param_t())
        :	IIModel(param), m_delta(1) {
          reset(param);
        }

      // Destructor
      virtual ~MBxxxLBPModel() {}

      // Clone the object
      virtual rmodel_t clone() const { return rmodel_t(new MBxxxLBPModel(*this)); }

      // Reset to new parameters
      virtual void reset(const param_t& param)
      {
        IIModel::reset(param);                  
        _reset();
      }

      // Project the selected features to a higher resolution
      virtual bool project() 
      {
        return _project();        
      }

      // Compute the value of the feature <f> at the (x, y) position
      virtual index_t get(index_t f, int x, int y) const
      {
        const mb_t& mb = m_mbs[f];
        return TLBPOp(m_iimage, x + mb.m_dx, y + mb.m_dy, mb.m_cx, mb.m_cy);
      }

      // Access functions
      virtual index_t n_features() const { return m_mbs.size(); }
      virtual index_t n_fvalues() const { return NFeatureValues; }

      // Describe a feature
      virtual string_t describe(index_t f) const
      {
        const mb_t& mb = m_mbs[f];
        return  "MB-" + LBPNames[TNameIndex] +
          " size "    +   boost::lexical_cast<string_t>((int)mb.m_cx * 3) + "x"
          +   boost::lexical_cast<string_t>((int)mb.m_cy * 3) + " @("
          +   boost::lexical_cast<string_t>((int)mb.m_dx) + ", "
          +   boost::lexical_cast<string_t>((int)mb.m_dy) + ")";
      }

      // Save/load specific (feature) information
      virtual void save(boost::archive::text_oarchive& oa) const
      {
        oa & m_mbs;
      }
      virtual void save(boost::archive::binary_oarchive& oa) const
      {
        oa & m_mbs;
      }
      virtual void load(boost::archive::text_iarchive& ia)
      {
        ia & m_mbs;
      }
      virtual void load(boost::archive::binary_iarchive& ia)
      {
        ia & m_mbs;
      }

    public:

      /////////////////////////////////////////////////////////////////////////////////////////
      // Multi-block feature parametrization:
      //	(dx, dy) displacement in the SW
      //	(cx, cy) cell size
      /////////////////////////////////////////////////////////////////////////////////////////

      struct mb_t
      {
        mb_t(int dx = 0, int dy = 0, int cx = 0, int cy = 0)
          :	m_dx(dx), m_dy(dy), m_cx(cx), m_cy(cy)
        {
        }

        friend class boost::serialization::access;
        template <typename Archive>
          void serialize(Archive& ar, const unsigned int)
          {
            ar & m_dx;
            ar & m_dy;
            ar & m_cx;
            ar & m_cy;
          }

        u_int8_t	m_dx, m_dy, m_cx, m_cy;
      };
      typedef std::vector<mb_t>	mbs_t;

    private:

      // Reset to new parameters
      void _reset()
      {
        m_delta = 0x01 << m_param.m_projections;

        // Build the MB features                                                
        const int min_cx = 1, max_cx = m_param.m_cols / 3;
        const int min_cy = 1, max_cy = m_param.m_rows / 3;

        m_mbs.clear();
        for (int cx = min_cx; cx <= max_cx; cx ++)
          for (int cy = min_cy; cy <= max_cy; cy ++)
          {
            const int min_x = 0, max_x = (int)m_param.m_cols - 3 * cx;
            const int min_y = 0, max_y = (int)m_param.m_rows - 3 * cy;

            const int ddx = 1, ddy = 1;                                

            // ... at each location
            for (int dx = min_x; dx < max_x; dx += ddx)
              for (int dy = min_y; dy < max_y; dy += ddy)
              {
                // ... but make sure it is projected at the correct scale
                if (    (dx % m_delta) == 0 &&
                    (dy % m_delta) == 0 &&        
                    (cx % m_delta) == 0 &&
                    (cy % m_delta) == 0)
                {
                  m_mbs.push_back(mb_t(dx, dy, cx, cy));					
                }
              }
          }       
      }

      // Project the selected features to a higher resolution
      bool _project() 
      {
        if (m_delta <= 1)
        {
          return false;
        }                        
        m_delta >>= 1;

        log_info("MBxLBPModel", "project")
          << "Projecting the selected features using the <" << m_delta << ">px resolution.\n";

        const indices_t features = IIModel::features();                         
        mbs_t old_mbs = m_mbs;

        // Project each feature ...
        m_mbs.clear();
        for (index_t ff = 0; ff < features.size(); ff ++)
        {
          const index_t f = features[ff];
          const mb_t& mb = old_mbs[f];

          const int min_cx = (int)mb.m_cx - m_delta, max_cx = min_cx + 2 * m_delta;
          const int min_cy = (int)mb.m_cy - m_delta, max_cy = min_cy + 2 * m_delta;

          const int centerx = mb.m_dx + 3 * mb.m_cx / 2;
          const int centery = mb.m_dy + 3 * mb.m_cy / 2;

          // ... by keeping its center fixed and scaling the cell size                                
          for (int cx = min_cx; cx <= max_cx; cx += m_delta)
          {
            for (int cy = min_cy; cy <= max_cy; cy += m_delta)
            {
              const int dx = centerx - 3 * cx / 2;
              const int dy = centery - 3 * cy / 2;
              if (    cx >= 0 && dx >= 0 && dx + 3 * cx < (int)m_param.m_cols &&
                  cy >= 0 && dy >= 0 && dy + 3 * cy < (int)m_param.m_rows)
              {
                bool found = false;
                for (index_t i = 0; i < m_mbs.size() && found == false; i ++)
                {
                  found = 
                    (dx == m_mbs[i].m_dx) &&
                    (dy == m_mbs[i].m_dy) &&
                    (cx == m_mbs[i].m_cx) &&
                    (cy == m_mbs[i].m_cy);
                }

                if (found == false)
                {
                  m_mbs.push_back(mb_t(dx, dy, cx, cy));
                }
              }
            }
          }
        }

        // OK
        return true;
      }

    protected:

      // Attributes
      mbs_t           m_mbs;		// MB-features
      index_t         m_delta;        // Projection level
  };

  // xLBP feature pools        
  typedef MBxxxLBPModel<mb_lbp<igrey_t, index_t>, 0, 256>                 MBLBPModel;
  typedef MBxxxLBPModel<mb_mlbp<igrey_t, index_t>, 1, 256>                MBmLBPModel;        
  typedef MBxxxLBPModel<mb_tlbp<igrey_t, index_t>, 2, 256>                MBtLBPModel;
  typedef MBxxxLBPModel<mb_dlbp<igrey_t, index_t>, 3, 256>                MBdLBPModel;                

  // MCT feature pool
  typedef MBxxxLBPModel<mb_mct<igrey_t, 3, 3, index_t>, 4, 512>           MBMCTModel;                

}}

#endif // BOB_VISIONER_MB_LBP_MODEL_H
