#pragma once

#include "mat_to_gl.h"

#include <GLFW/glfw3.h>
#include <opencv2/opencv.hpp>
#include <string>

class window_glfw
{
public:
	window_glfw(unsigned width, unsigned height, std::string title)
	{
		if (!glfwInit())
		{
			_good = false;
			return;
		}

		_w = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
		if (!_w)
		{
			_good = false;
			return;
		}

		glfwMakeContextCurrent(_w);

		init_opengl(width, height);
	}

	~window_glfw()
	{
		if (_w)
			glfwDestroyWindow(_w);
		glfwTerminate();
	}

	bool is_good() const
	{
		return _good;
	}

	bool should_close() const
	{
		return glfwWindowShouldClose(_w);
	}

	void show(const cv::Mat& img, unsigned delay)
	{
		cimbar::mat_to_gl::draw(img);
		swap();
		poll();
	}

protected:
	void init_opengl(int width, int height)
	{
		glViewport(0, 0, width, height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0, width, height, 0.0, 0.0, 100.0);
	}

	void swap()
	{
		// show next frame
		glfwSwapBuffers(_w);
	}

	void poll()
	{
		glfwPollEvents();
	}

protected:
	GLFWwindow* _w;
	bool _good = true;
};
