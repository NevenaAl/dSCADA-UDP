using System;

namespace WpfDSCADA.SVG
{
    public class SVGLayer
    {
        public int layer_priority;
        public String layer_name;
        public int layer_index;
        public bool layer_active;
        //public List<SVGShape> layer_shape_list;

        /// <summary>
        /// Constructor
        /// </summary>        
        public SVGLayer()
        {
            layer_priority = -1;
            layer_name = "";
            layer_index = -1;
            layer_active = false;
        }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="priority"></param>
        /// <param name="name"></param>
        /// <param name="index"></param>
        /// <param name="active"></param>
        public SVGLayer(int priority, String name, int index, bool active)
        {
            layer_priority = priority;
            layer_name = name;
            layer_index = index;
            layer_active = active;
        }

        /// <summary>
        /// Set Methods
        /// </summary>        
        public void SetLayerPriority(int var)
        {
            layer_priority = var;
        }

        public void SetLayerName(String set_layer_name)
        {
            layer_name = set_layer_name;
        }

        public void SetLayerIndex(int var)
        {
            layer_index = var;
        }

        public void SetLayerActive(bool var)
        {
            layer_active = var;
        }

        /// <summary>
        /// Set Methods
        /// </summary>   
        public int GetLayerPriority()
        {
            return layer_priority;
        }

        public int GetLayerIndex()
        {
            return layer_index;
        }

        public bool GetLayerActive()
        {
            return layer_active;
        }

        public String GetLayerName()
        {
            return layer_name;
        }
    }
}
