using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace CCGUI
{
	public class UITextureManager
	{
		Dictionary<string, outki.Atlas> m_mapping = new Dictionary<string, outki.Atlas>();

		public UITextureManager()
		{

		}

		public void AddAtlas(outki.Atlas atlas)
		{
			foreach (outki.AtlasOutput ao in atlas.Outputs)
			{
				foreach (outki.AtlasEntry entry in ao.Entries)
				{
					m_mapping[entry.id] = atlas;
				}
				break;
			}
		}

		// id is path really.
		public UIRenderer.Texture ResolveTexture(outki.Texture tex, float pixelScale, float u0, float v0, float u1, float v1, bool uvInTexels = false)
		{
			if (m_mapping.ContainsKey(tex.id))
			{
				outki.Atlas a = m_mapping[tex.id];

				// Pick the most closely scaled version of the atlas.
				double bestMatch = 100000;
				outki.AtlasOutput pickedAtlas = null;
				foreach (outki.AtlasOutput output in a.Outputs)
				{
					double diff = Math.Abs(1 - output.Scale / pixelScale);
					if (diff < bestMatch)
					{
						bestMatch = diff;
						pickedAtlas = output;
					}
				}

				if (uvInTexels)
				{
					// u/v are in original texture coordinates.
					u0 = u0 / tex.Width;
					u1 = u1 / tex.Width;
					v0 = v0 / tex.Height;
					v1 = v1 / tex.Height;
				}

				foreach (outki.AtlasEntry entry in pickedAtlas.Entries)
				{
					if (entry.id == tex.id)
					{
						float uvw = entry.u1 - entry.u0;
						float uvh = entry.v1 - entry.v0;

						float _u0 = entry.u0 + uvw * u0;
						float _v0 = entry.v0 + uvh * v0;
						float _u1 = _u0 + uvw * (u1 - u0);
						float _v1 = _v0 + uvh * (v1 - v0);

						return UIRenderer.ResolveTextureUV(pickedAtlas.Texture, _u0, _v0, _u1, _v1);
					}
				}

				return null;
			}
			else
			{
				return UIRenderer.ResolveTexture(tex);
			}
		}
	}
}
