#pragma once

#include "CADRMXTypeDef.h"
#include <tuple>

// Forward declarations
class ISDRmcInstance;

namespace RMXLib
{
	enum RMX_WATERMARK_FONTSTYLE {
		FontStyle_Regular = 0,
		FontStyle_Bold = 1,
		FontStyle_Italic = 2,
		FontStyle_BoldItalic = 3
	};

	enum RMX_WATERMARK_TEXTALIGNMENT {
		TextAlign_Left = 0,
		TextAlign_Centre = 1,
		TextAlign_Right = 2
	};

	struct RMX_WATERMARK_COLOR
	{
		unsigned char colorA;
		unsigned char colorR;
		unsigned char colorG;
		unsigned char colorB;
	};

	struct RMX_WATERMARK_INFO
	{
		std::wstring text;
		RMX_WATERMARK_TEXTALIGNMENT textAlignment;
		std::wstring fontName;
		int fontSize;
		int fontRotation;
		RMX_WATERMARK_FONTSTYLE fontStyle;
		RMX_WATERMARK_COLOR fontColor;
	};

	/*!
	* Set view overlay for the given window
	*
	* \param pDRmInstance: Pointer to SkyDRM instance
	* \param hTargetWnd: Target window to display view overlay
	* \param tags: File tags to be evaluated by centre policy. Able to specify multiple json-style strings as a vector
	*				in case multiple files need to be evaluated.
	* \param ctxAttrs: Additional attributes to be evaluated by centre policy. Could be context related.
	* \return bool: False if call is failure
	* */
	bool SetViewOverlay(
		ISDRmcInstance* pDRmInstance,
		void* hTargetWnd,	
		const std::vector<std::string>& tags,
		const TEvalAttributeList& ctxAttrs,
		const std::tuple<int, int, int, int>& displayOffset = { 0,0,0,0 } /*{left,upper,right,bottom}*/);

	/*!
	* Set view overlay for the given window
	*
	* \param pDRmInstance: Pointer to SkyDRM instance
	* \param hTargetWnd: Target window to display view overlay
	* \param nxlFiles: Files to be evaluated by centre policy. FIFO rule means the first file with watermark found will be taken into account.
	* \param ctxAttrs: Additional attributes to be evaluated by centre policy. Could be context related.
	* \return bool: False if call is failure
	* */
	bool SetViewOverlay(
		ISDRmcInstance* pDRmInstance,
		void* hTargetWnd,
		const std::vector<std::wstring>& nxlFiles,
		const TEvalAttributeList& ctxAttrs,
		const std::tuple<int, int, int, int>& displayOffset = { 0,0,0,0 } /*{left,upper,right,bottom}*/);

	/*!
	 * Remove the view overlay for the given window
	 * \param pDRmInstance: Pointer to SkyDRM instance
	 * \param hTargetWnd: Target window to clear 
	 * \return bool: False if call is failure
	 */
	bool ClearViewOverlay(
		ISDRmcInstance* pDRmInstance,
		void* hTargetWnd);

	/*!
	 * Evaluate watermark information defined in the nxl file
	 * \param pDRmInstance: Pointer to SkyDRM instance
	 * \param nxlFile: Nxl file to be evalauted
	 * \param ctxAttrs: Additional attributes to be evaluated by centre policy. Could be context related.
	 * \param wmInfo: Return watermark infomation
	 * \return bool: False if call is failure
	 */
	bool EvalWatermarkInfo(
		ISDRmcInstance* pDRmcInst,
		const std::wstring& nxlFile,
		const TEvalAttributeList& ctxAttrs,
		RMX_WATERMARK_INFO& wmInfo);

}; // namespace RMXLib

