using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;


[RequireComponent(typeof(Camera))]
public class StereoBlit : MonoBehaviour {
    public Camera lcam;
    public Camera rcam;

    private Camera cam;

    private RenderTexture ltex;
    private RenderTexture rtex;


	[DllImport ("RenderingPlugin")]
	private static extern void SetTimeFromUnity(float t);

	[DllImport ("RenderingPlugin")]
	private static extern void SetTextureFromUnity(System.IntPtr ltex, System.IntPtr rtex, int w, int h);

	[DllImport("RenderingPlugin")]
	private static extern IntPtr GetRenderEventFunc();

	IEnumerator Start()
	{
        cam = GetComponent<Camera>();
        cam.depth = 100;

        ltex = new RenderTexture(lcam.pixelWidth, lcam.pixelHeight, 24, RenderTextureFormat.ARGB32);
        ltex.Create();
        lcam.targetTexture = ltex;
        lcam.Render();

        rtex = new RenderTexture(rcam.pixelWidth, rcam.pixelHeight, 24, RenderTextureFormat.ARGB32);
        rtex.Create();
        rcam.targetTexture = rtex;
        rcam.Render();

        SetTextureFromUnity (ltex.GetNativeTexturePtr(), rtex.GetNativeTexturePtr(), ltex.width, ltex.height);

		yield return StartCoroutine("CallPluginAtEndOfFrames");
	}

    private void OnRenderImage(RenderTexture source, RenderTexture destination) {
        RenderTexture current = RenderTexture.active;
	    if(cam.stereoActiveEye == Camera.MonoOrStereoscopicEye.Left) {
            Graphics.Blit(lcam.targetTexture, destination);
        } else if (cam.stereoActiveEye == Camera.MonoOrStereoscopicEye.Right) {
            Graphics.Blit(rcam.targetTexture, destination);
        }
        RenderTexture.active = current;
    }

    private IEnumerator CallPluginAtEndOfFrames()
	{
		while (true) {
			yield return new WaitForEndOfFrame();

            SetTimeFromUnity (Time.timeSinceLevelLoad);
            
			GL.IssuePluginEvent(GetRenderEventFunc(), 1);
		}
	}
}
