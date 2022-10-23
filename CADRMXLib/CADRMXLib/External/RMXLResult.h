//
// This is a part of the NextLabs CADRMX Client SDK.
// Copyright (C) 2020 NextLabs, Inc. All Rights Reserved.
//

#pragma once

#include <string>
#include "RMXLTypeDef.h"

/** Error code  */
typedef unsigned long RMXErrCode_t;

/*!
* \file RMXLResult.h
 * \class RMXResult
 * 
 * This class provides information about the result of API call.
 */
class RMXResult
{
public:
	/** Default constructor */
	RMXResult() : m_code(0) {};

	/**
	 * Constructor
	 *
	 * @param  [in] code 0 if succeeded. Otherwise, error code. 
	 * @param  [in] errMessage  Error message if failed.
	 */

	explicit RMXResult(RMXErrCode_t code, const std::wstring& errMessage)
		: m_code(code), m_message(errMessage)
	{
	}
	/** Destructor */
	virtual ~RMXResult() {}

	/**
	 * Copy constructor
	 *
	 */

	RMXResult(const RMXResult& other)
		: m_code(other.m_code), m_message(other.m_message)
	{
	}

	/**
	 * Assignment operator
	 * 
	 */

	RMXResult& operator = (const RMXResult& other)
	{
		if (this != &other)
		{
			m_code = other.m_code;
			m_message = other.m_message;
		}

		return *this;
	}

	/**
	 * Operator to check if the result indicates failure with error
	 *
	 * @returns True if no error occurs.
	 */

	operator bool() const { return (0 == m_code); }

	/**
	 * Operator to check if they are same error
	 *
	 * \param  code The error code.
	 *
	 * \returns True if error code is same.
	 */

	bool operator == (int code) const { return (code == m_code); }

	/**
	 * Operator to check if they are same error
	 *
	 * \param  code The code.
	 *
	 * \returns False if error code is same.
	 */
	bool operator != (int code) const { return (code != m_code); }

	/**
	 * Gets error code
	 *
	 * \returns The error code.
	 */

	RMXErrCode_t GetErrCode() const { return m_code; }

	/**
	 * Gets error message
	 *
	 * \returns The error message.
	 */

	std::wstring GetErrMessage() const { return m_message; }

protected:
	/** The code */
	RMXErrCode_t m_code;
	/** The message */
	std::wstring m_message;
};

