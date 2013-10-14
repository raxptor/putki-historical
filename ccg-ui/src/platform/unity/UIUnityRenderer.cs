using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class UIRenderer
{
	public struct RColor
	{
		public RColor(float _r, float _g, float _b, float _a)
		{
			r = _r;
			g = _g;
			b = _b;
			a = _a;
		}
		
		public float r, g, b, a;
	}
	
	public static RColor m_currentColor = new RColor(1,1,1,1);
	public static int begin = -1;
	public static Shader TexturedShader = null;
	public static Shader SolidShader = null;
	public static Material SolidMaterial = null;
	
	public class LoadedTexture
	{
		public string path;
		public Texture2D unityTexture;
		public Material material;
	};
	
	public class Texture
	{
		public LoadedTexture ld;
		public float u0, v0, u1, v1;
	}
	
	static List<LoadedTexture> loaded = new List<LoadedTexture>();
	
	public static LoadedTexture lastTexture = null;
	
	public static void SetColor(RColor c)
	{
		m_currentColor = c;
	}
	
	public static RColor GetColor(RColor c)
	{
		return m_currentColor;
	}
	
	public static void MultipylColor(RColor c)
	{
		m_currentColor.r *= c.r;
		m_currentColor.g *= c.g;
		m_currentColor.b *= c.b;
		m_currentColor.a *= c.a;
	}

	public static Texture ResolveTexture(outki.Texture tex)
	{	
		return ResolveTextureUV(tex, 0, 0, 1, 1);
	}
	
	public static LoadedTexture LoadTexture(outki.Texture tex)
	{
		outki.TextureOutputPng png = (outki.TextureOutputPng) tex.Output;
		if (png != null)
		{
			foreach (LoadedTexture lt in loaded)
			{
				if (lt.path == png.PngPath)
				{
					return lt;
				}
			}
			
			TextAsset ta = Resources.Load(png.PngPath.Replace("Resources/",""), typeof(TextAsset)) as TextAsset;
			if (ta == null)
			{
				UnityEngine.Debug.LogError("Failed to load texture [" + png.PngPath + "]");
				return null;
			}

			LoadedTexture ld = new LoadedTexture();
			ld.unityTexture = new Texture2D (4, 4);
			ld.unityTexture.LoadImage(ta.bytes);
			ld.unityTexture.filterMode = FilterMode.Bilinear;
			ld.unityTexture.anisoLevel = 0;
			ld.material = new Material(TexturedShader);
			ld.material.mainTexture = ld.unityTexture;
			ld.path = png.PngPath;
			loaded.Add(ld);
			
			UnityEngine.Debug.Log("Loaded texture " + png.PngPath + " it is "+ ld.unityTexture);		
			return ld;
		}
		
		return null;
	}
	
	public static Texture ResolveTextureUV(outki.Texture tex, float u0, float v0, float u1, float v1)
	{
		UIRenderer.Texture t = new Texture();
		t.ld = LoadTexture(tex);
		t.u0 = u0;
		t.v0 = v0;
		t.u1 = u1;
		t.v1 = v1;
		return t;
	}
	
	public static void DrawTexture(Texture tex, float x0, float y0, float x1, float y1)
	{
		DrawTextureUV(tex, x0, y0, x1, y1, tex.u0, tex.v0, tex.u1, tex.v1);
	}
		
	public static void DrawTextureUV(Texture tex, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1)
	{		
		if (tex.ld != lastTexture || begin == 0)
		{
			if (begin != 0)
				GL.End();
			
			lastTexture = tex.ld;
			lastTexture.material.SetPass(0);
			GL.Begin(GL.QUADS);
			begin = 1;
		}
		
		GL.Color(new Color(m_currentColor.r, m_currentColor.g, m_currentColor.b, m_currentColor.a));
		GL.TexCoord2(u0, 1 - v0);
		GL.Vertex3(x0, y0, 0);
		GL.TexCoord2(u1, 1 - v0);
		GL.Vertex3(x1, y0, 0);
		GL.TexCoord2(u1, 1 - v1);
		GL.Vertex3(x1, y1, 0);
		GL.TexCoord2(u0, 1 - v1);
		GL.Vertex3(x0, y1, 0);
	}
	
	public static void DrawSolidRect(float x0, float y0, float x1, float y1, outki.UIColor col = null)
	{
		if (lastTexture != null || begin == 0)
		{
			if (begin != 0)
				GL.End();
			
			lastTexture = null;
			SolidMaterial.SetPass(0);
			GL.Begin(GL.QUADS);
			begin = 1;
		}

		if (col == null)
			GL.Color(new Color(m_currentColor.r, m_currentColor.g, m_currentColor.b, m_currentColor.a));
		else
			GL.Color(new Color(m_currentColor.r * col.r / 255, m_currentColor.g * col.g / 255.0f, m_currentColor.b * col.b / 255.0f, m_currentColor.a * col.a / 255.0f));

		GL.Vertex3(x0, y0, 0);
		GL.Vertex3(x1, y0, 0);
		GL.Vertex3(x1, y1, 0);
		GL.Vertex3(x0, y1, 0);
	}
	
	public static void HelpGLColor(outki.UIColor t)
	{
		GL.Color(new Color(m_currentColor.r * t.r / 255, m_currentColor.g * t.g / 255.0f, m_currentColor.b * t.b / 255.0f, m_currentColor.a * t.a / 255.0f));		
	}
	
	public static void DrawGradientRect(float x0, float y0, float x1, float y1, 
	                                    outki.UIColor tl, outki.UIColor tr,
	                                    outki.UIColor bl, outki.UIColor br)
	
	{
		if (lastTexture != null || begin == 0)
		{
			if (begin != 0)
				GL.End();
			
			lastTexture = null;
			SolidMaterial.SetPass(0);
			GL.Begin(GL.QUADS);
			begin = 1;
		}

		HelpGLColor(tl);
		GL.Vertex3(x0, y0, 0);
		HelpGLColor(tr);
		GL.Vertex3(x1, y0, 0);
		HelpGLColor(br);
		GL.Vertex3(x1, y1, 0);
		HelpGLColor(bl);
		GL.Vertex3(x0, y1, 0);
	}	

	public static void Begin()
	{
		if (SolidMaterial == null && SolidShader != null)
		{
			SolidMaterial =	new Material(SolidShader);
		}
		
		m_currentColor.r = m_currentColor.g = m_currentColor.b = m_currentColor.a = 1.0f;
		GL.LoadPixelMatrix(0, Screen.width, Screen.height, 0);
	}
	
	public static void End()
	{
		if (begin != 0)
		{
			GL.End();
			begin = 0;
		}
	}
}
