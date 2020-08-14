#include "Framework.h"

Kaya::Kaya()
	: moveSpeed(30.0f), rotSpeed(2.0f), state(IDLE), accelation(10.0f),
	deceleration(3.0f), velocity(0, 0, 0), aStar(nullptr)
{
	model = NEW ModelAnimator(L"ModelAnimationInstancing");
	transform = model->AddTransform();
	ReadData();
	model->SetEndEvent(ATTACK, bind(&Kaya::SetIdle, this));

	model->PlayClip(0, state);

	transform->scale = Vector3(0.05f, 0.05f, 0.05f);
	transform->position = { 100.0f,0.0f,100.0f };
	headTransform = NEW Transform();
//	headTransform->SetParent(&headBone->transform);
	transform->SetRotPivot(0 , 7, 0);

	LightInfo info;
	info.type = LightInfo::SPOT;
	info.color = Float4(0.6f, 1, 0.6f, 1);
	info.inner = 20.0f;
	info.outer = 50.0f;
	info.range = 30.0f;
	info.power = 20.0f;

	light_info_num = LIGHT->Add(info);
}

Kaya::~Kaya()
{
	delete model;
	delete headTransform;
}

void Kaya::Update()
{
	Input();
	Move();
	Rotate();

	transform->position.SetY(terrain->GetHeight(transform->position));
	vector<ModelBone*>* bones = model->GetModel()->GetBones();
	ModelMesh* mesh;
	
	for (UINT b = 0; b < bones->size(); b++)
	{
		ModelBone* bone = bones->at(b);
		if (bone->name == "mixamorig:RightHand")
		{
			headBone = bone;
			break;
		}
	}
	headTransform->SetTransform(headBone->transform);
	headTransform->UpdateWorld();
	model->Update();
	LIGHT->data.lights[light_info_num].position = CAMERA->position;
	LIGHT->data.lights[light_info_num].direction = CAMERA->GetForward();
}

void Kaya::Render()
{
	model->Render();
}

void Kaya::Input()
{
	if (KEY_PRESS('W'))
		velocity -= transform->GetForward() * accelation * DELTA;
	if (KEY_PRESS('S'))
		velocity += transform->GetForward() * accelation * DELTA;

	if (KEY_PRESS('A'))
		velocity += transform->GetRight() * accelation * DELTA;
	if (KEY_PRESS('D'))
		velocity -= transform->GetRight() * accelation * DELTA;

	if (KEY_PRESS(VK_RBUTTON))
		SetAnimation(ATTACK);
	if (aStar != nullptr)
	{

		if (KEY_DOWN(VK_LBUTTON))
		{
			path.clear();

			terrain->ComputePicking(&destPos);

			Ray ray;
			ray.position = transform->position;
			ray.direction = (destPos - transform->position).Normal();

			float distance = Distance(ray.position, destPos);

			if (aStar->IsCollisionObstacle(ray, distance))
			{
				int startIndex = aStar->FindCloseNode(transform->position);
				int endIndex = aStar->FindCloseNode(destPos);

				aStar->Reset();

				path = aStar->FindPath(startIndex, endIndex);

				aStar->MakeDirectPath(transform->position, destPos, path);

				path.insert(path.begin(), destPos);

				int pathSize = path.size();

				while (path.size() > 2)
				{
					vector<Vector3> tempPath;
					for (int i = 1; i < path.size() - 1; i++)
						tempPath.push_back(path[i]);

					Vector3 start = path.back();
					Vector3 end = path.front();

					aStar->MakeDirectPath(start, end, tempPath);

					path.clear();
					path.push_back(end);

					for (Vector3 temp : tempPath)
						path.push_back(temp);

					path.push_back(start);

					if (pathSize == path.size())
						break;
					else
						pathSize = path.size();
				}
			}
			else
			{
				path.push_back(destPos);
			}
		}
	}

	//XMFLOAT4X4 mat;
	//XMStoreFloat4x4(&mat,headBone->transform);

	
	
}

void Kaya::Move()
{
	float padding = 5.0f;
	if (state == ATTACK)
		return;

	if (aStar != nullptr)
	{
		MovePath();
	}

	float magnitude = velocity.Length();

	if (magnitude > 1.0f)
		velocity.Normalize();

	if (magnitude > 0.1f)
	{
		transform->position += velocity * moveSpeed * DELTA;

		
		SetAnimation(RUN);

		velocity = XMVectorLerp(velocity.data, XMVectorZero(), deceleration * DELTA);
	}
	else
	{
		SetAnimation(IDLE);
	}

	
}

void Kaya::MovePath()
{
	if (path.empty())
		return;

	Vector3 dest = path.back();

	Vector3 direction = dest - transform->position;
	velocity = direction.Normal();

	if (direction.Length() < 0.3f)
		path.pop_back();
}

void Kaya::Rotate()
{
	if (velocity.Length() < 0.1f)
		return;

	//transform->rotation.x = CAMERA->rotation.x;
	
	Vector3 start = transform->GetForward();
	Vector3 end = velocity.Normal();

	float cosValue = start.Dot(end);
	float angle = acos(cosValue);

	if (angle < 0.1f)
		return;

	Vector3 cross = start.Cross(end);

	if (cross.GetY() > 0.0f)
		transform->rotation -= kUp * DELTA * rotSpeed;
	else
		transform->rotation += kUp * DELTA * rotSpeed;
}

void Kaya::SetAnimation(AnimState value)
{
	if (state != value)
	{
		state = value;
		model->PlayClip(0, value);
	}
}

void Kaya::SetIdle()
{
	SetAnimation(IDLE);
}

void Kaya::ReadData()
{
	string name = "Swat";


	ModelReader* reader = NEW ModelReader();
	reader->ReadFile("ModelData/Models/" + name + ".fbx");
	//reader->ExportMaterial(name + "/" + name);
	reader->ExportMesh(name + "/" + name);
	delete reader;

	reader = NEW ModelReader();
	reader->ReadFile("ModelData/Animations/" + name + "/Rifle_Idle.fbx");
	reader->ExportAnimClip(0, name + "/Rifle_Idle");
	delete reader;

	reader = NEW ModelReader();
	reader->ReadFile("ModelData/Animations/" + name + "/Rifle_Run.fbx");
	reader->ExportAnimClip(0, name + "/Rifle_Run");
	delete reader;

	reader = NEW ModelReader();
	reader->ReadFile("ModelData/Animations/" + name + "/Rifle_Attack.fbx");
	reader->ExportAnimClip(0, name + "/Rifle_Attack");
	delete reader;

	string path = name + "/" + name;
	model->ReadMaterial(path);
	model->ReadMesh(path);
	model->ReadClip(name + "/Rifle_Idle");
	model->ReadClip(name + "/Rifle_Run");
	model->ReadClip(name + "/Rifle_Attack");

	/*model->ReadClip(name + "/Capoeira");
	model->ReadClip(name + "/Idle");
	model->ReadClip(name + "/Running");
	model->ReadClip(name + "/Attack");*/
}
