/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "FountainEncoder.h"
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

class fountain_encoder_stream
{
public:
	static const unsigned _headerSize = 6;

protected:
	fountain_encoder_stream(std::string&& data, unsigned buffer_size, uint8_t encode_id)
	    : _data(data)
	    , _buffer(buffer_size, 0)
	    , _encodeId(encode_id)
	    , _encoder((uint8_t*)_data.data(), _data.size(), block_size())
	{
	}

	unsigned block_size() const
	{
		return _buffer.size() - _headerSize;
	}

public:
	template <typename STREAM>
	static fountain_encoder_stream create(STREAM& stream, unsigned buffer_size, uint8_t encode_id=0)
	{
		std::stringstream buffs;
		if (stream)
			 buffs << stream.rdbuf();
		return fountain_encoder_stream(buffs.str(), buffer_size, encode_id);
	}

	template <typename STREAM>
	static std::shared_ptr<fountain_encoder_stream> create_shared(STREAM& stream, unsigned buffer_size, uint8_t encode_id=0)
	{
		std::stringstream buffs;
		if (stream)
			buffs << stream.rdbuf();
		return std::shared_ptr<fountain_encoder_stream>( new fountain_encoder_stream(buffs.str(), buffer_size, encode_id) );
	}

	bool good() const
	{
		return _encoder.good();
	}

	unsigned block_count() const
	{
		return _block;
	}

	unsigned blocks_required() const
	{
		return (_data.size() / block_size()) + 1;
	}

	void encode_new_block()
	{
		unsigned char* data = _buffer.data() + _headerSize;
		size_t res = _encoder.encode(_block++, data, block_size());
		if (res != block_size())
			_encoder.encode(_block++, data, block_size()); // try twice -- the last initial block will be the wrong size

		// write header
		_buffer.data()[0] = _encodeId;
		_buffer.data()[1] = (_data.size() >> 16) & 0xFF;
		_buffer.data()[2] = (_data.size() >> 8) & 0xFF;
		_buffer.data()[3] = _data.size() & 0xFF;

		unsigned block = _block - 1; // we already incremented it above
		_buffer.data()[4] = (block >> 8) & 0xFF;
		_buffer.data()[5] = block & 0xFF;
		_buffIndex = 0;
	}

	// sometimes we want a new encoded batch, sometimes we just want our buffer
	fountain_encoder_stream& read(char* data, unsigned length)
	{
		std::streamsize totalRead = 0;
		while (length > 0 and good())
		{
			if (_buffIndex >= _buffer.size())
				encode_new_block();

			unsigned readLen = std::min(length, (unsigned)(_buffer.size() - _buffIndex));
			if (!readLen)
				break;
			totalRead += readLen;

			uint8_t* first = _buffer.data() + _buffIndex;
			std::copy(first, first + readLen, data);

			length -= readLen;
			data += readLen;
			_buffIndex += readLen;
		}
		_lastRead = totalRead;
		return *this;
	}

	std::streamsize readsome(char* data, unsigned length)
	{
		read(data, length);
		return gcount();
	}

	std::streamsize gcount() const
	{
		return _lastRead;
	}

protected:
	std::string _data;
	std::vector<uint8_t> _buffer;
	uint8_t _encodeId;
	FountainEncoder _encoder;

	unsigned _buffIndex = ~0U;
	unsigned _block = 0;
	std::streamsize _lastRead = 0;
};
