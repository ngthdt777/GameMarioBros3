#include "Pipe.h"
Pipe::Pipe(int model)
{
	this->model = model;

}

void Pipe::Render()
{
	int ani = -1;
	if (model == SHORT_PIPE)
		ani = ANI_SHORT_PIPE;
	if (model == MEDIUM_PIPE)
		ani = ANI_MEDIUM_PIPE;
	animation_set->at(ani)->Render(x, y);
}
void Pipe::GetBoundingBox(float& l, float& t, float& r, float& b)
{
	l = x;
	t = y;
	r = x + PIPE_BBOX_WIDTH;
	if (model == SHORT_PIPE)
	{
		b = y + PIPE_SHORT_BBOX_HEIGHT;
	}
	if (model == MEDIUM_PIPE)
	{
		b = y + PIPE_MEDIUM_BBOX_HEIGHT;
	}
}
Pipe::~Pipe()
{
}
