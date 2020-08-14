#include "Framework.h"
#include "FPSModel.h"

float FPSModel::hp = 0;
float FPSModel::money = 0;
Vector3 FPSModel::position = Vector3(0, 0, 0);


FPSModel::FPSModel()
	:move_speed(20.0f), bullet_left_num(10), max_hp(100), cur_weapon_num(Weapon::PISTOL)
{
	level = 0;
	hp = max_hp;
	atk = 1.f;
	is_atk = false;
	is_muzzle = false;

	for (UINT i = 0; i < 4; i++)
	{
		Weapon weaponIndex = static_cast<Weapon>(i);
		switch (weaponIndex)
		{
		case PISTOL:
			weapon_atk[weaponIndex] = 3;
			weapon_num_charge[weaponIndex] = 15;
			weapon_bullet_left_num[weaponIndex] = weapon_num_charge[weaponIndex];
			weapon_bullet_num[weaponIndex] = 50;
			weapon_reload_time[weaponIndex] = 3.5f;
			weapon_shot_time[weaponIndex] = 0.35f;
			weapon_rebound[weaponIndex] = 0.02f;
			break;

		case RIFLE:
			weapon_atk[weaponIndex] = 2;
			weapon_num_charge[weaponIndex] = 30;
			weapon_bullet_left_num[weaponIndex] = weapon_num_charge[weaponIndex];
			weapon_bullet_num[weaponIndex] = 100;
			weapon_reload_time[weaponIndex] = 3.5f;
			weapon_shot_time[weaponIndex] = 0.08f;
			weapon_rebound[weaponIndex] = 0.02f;
			break;

		case SHOTGUN:
			weapon_atk[weaponIndex] = 10;
			weapon_num_charge[weaponIndex] = 7;
			weapon_bullet_left_num[weaponIndex] = weapon_num_charge[weaponIndex];
			weapon_bullet_num[weaponIndex] = 30;
			weapon_reload_time[weaponIndex] = 1.1f;
			weapon_shot_time[weaponIndex] = 2.0f;
			weapon_rebound[weaponIndex] = 0.03f;
			break;

		case SNIP_RIFLE:
			weapon_atk[weaponIndex] = 13;
			weapon_num_charge[weaponIndex] = 5;
			weapon_bullet_left_num[weaponIndex] = weapon_num_charge[weaponIndex];
			weapon_bullet_num[weaponIndex] = 10;
			weapon_reload_time[weaponIndex] = 3.5f;
			weapon_shot_time[weaponIndex] = 1.5f;
			weapon_rebound[weaponIndex] = 0.05f;
			break;
		default:;
		}
	}
	SetModel();
	model_transform->scale = { 0.05f,0.05f,0.05f };
	SOUND->Add("gun_reload", "Assets/Sounds/reload.mp3", false);
	SOUND->Add("gun_reload2", "Assets/Sounds/reload2.ogg", false);
	SOUND->Add("gun_reload3", "Assets/Sounds/reload3.ogg", false);
	SOUND->Add("gun_r", "Assets/Sounds/r.mp3", false);
	SOUND->Add("gun_draw", "Assets/Sounds/draw.wav", false);

	SOUND->Add("gun_shot1", "Assets/Sounds/weapon-sound5.ogg", false);
	SOUND->Add("gun_shot2", "Assets/Sounds/weapon-sound19.ogg", false);
	SOUND->Add("gun_shot3", "Assets/Sounds/weapon-sound1.ogg", false);
	SOUND->Add("gun_shot4", "Assets/Sounds/weapon-sound15.ogg", false);

	SOUND->Add("move", "Assets/Sounds/step-sound9.ogg", false);
	SOUND->Add("cash", "Assets/Sounds/cash.mp3", false);
	SOUND->Add("beep", "Assets/Sounds/beep.wav", false);
	collider_transform = NEW Transform();
	collider_transform->position.y = 6.0f;

	hit_collider = NEW BoxCollider();
	hit_collider->SetTarget(collider_transform);
	hit_collider->scale = { 3.5f,2.0f,3.5f };
	obj_collider = NEW SphereCollider(3);
	obj_collider->SetTarget(collider_transform);
	CAMERA->SetTarget(model_transform);
	CAMERA->SetHeight(1.0f);
	CAMERA->SetOffset(Vector3(0.8f, 0, 1.5f));
	model_transform->SetRotPivot(0, 1.0f, 0);
	max_rot = 0.05f;
	min_rot = -0.05f;
	cur_rot = 0.0f;
	is_rot_up = true;
	is_move = false;
	is_dash = false;
	is_action = false;
	is_muzzle_ones = false;

	LightInfo info;
	info.type = LightInfo::SPOT;
	info.color = Float4(0.4f, 0.4f, 0.4f, 1);
	info.inner = 25.0f;
	info.outer = 60.0f;
	info.range = 100.0f;
	info.power = 16.0f;

	light_info_num = LIGHT->Add(info);

	info;
	info.type = LightInfo::CAPSULE;
	info.color = Float4(0.6f, 0.5f, 0.2f, 1);
	info.inner = 20.0f;
	info.outer = 30.0f;
	info.range = 10.0f;
	info.power = 10.f;
	info.length = 30.0f;

	muzzle_info_num = LIGHT->Add(info);


	move_sound_loop = NEW Loop(0.6f, false);
	shot_loop = NEW Loop(weapon_shot_time[cur_weapon_num], false);
	reload_sound_loop = NEW Loop(weapon_reload_time[cur_weapon_num], false);
	draw_loop = NEW Loop(1.2f, false);
	idle_loop = NEW Loop(3.0f, true);
	shotgun_reload_loop = NEW Loop(weapon_reload_time[Weapon::SHOTGUN], false);
	muzzle_loop = NEW Loop(0.02f, false);

	move_sound_loop->StartLoop();
	draw_loop->StartLoop();
	shot_loop->StartLoop();
	reload_sound_loop->StartLoop();
	idle_loop->StartLoop();

	shot_loop->loopFunc = bind(&FPSModel::CheckActionsLoop, this);
	reload_sound_loop->loopFunc = bind(&FPSModel::CheckActionsLoop, this);
	draw_loop->loopFunc = bind(&FPSModel::CheckActionsLoop, this);
	idle_loop->loopFunc = bind(&FPSModel::IdleAction, this);
	shotgun_reload_loop->loopFunc = bind(&FPSModel::ShotGunReloadLoop, this);
	muzzle_loop->loopFunc = bind(&FPSModel::MuzzleLoop, this);
	Texture* temp = Texture::Add(L"Textures/hp_bar.png");
	hp_bar = NEW Render2D();
	hp_bar->scale = { 600,100,1 };
	hp_bar->position.x = 0.0f;
	hp_bar->position.y = 0;
	hp_bar->SetTexture(temp);

	temp = Texture::Add(L"Textures/hp_back.png");
	hp_back = NEW Render2D();
	hp_back->position.x = 0.f;
	hp_back->position.y = 0;
	hp_back->scale = { 600,100,1 };
	hp_back->SetTexture(temp);

	temp = Texture::Add(L"Textures/Attack.png");
	atk_up_icon = NEW Render2D();
	atk_up_icon->position.x = CENTER_X;
	atk_up_icon->position.y = 50.f;
	atk_up_icon->scale = { 100,100,1 };
	atk_up_icon->SetTexture(temp);

	temp = Texture::Add(L"Textures/heal.png");
	hp_up_icon = NEW Render2D();
	hp_up_icon->position.x = CENTER_X + 140;
	hp_up_icon->position.y = 50.f;
	hp_up_icon->scale = { 100,100,1 };
	hp_up_icon->SetTexture(temp);

	temp = Texture::Add(L"Textures/Magazine.png");
	bullet_up_icon = NEW Render2D();
	bullet_up_icon->position.y = 50.f;
	bullet_up_icon->position.x = CENTER_X - 140;
	bullet_up_icon->scale = { 100,100,1 };
	bullet_up_icon->SetTexture(temp);


	temp = Texture::Add(L"Textures/bullet_icon_g.png");
	for (UINT i = 0; i < 30; i++)
	{
		Render2D* bullet_icon = NEW Render2D();
		bullet_icon->position.x = WIN_WIDTH * 0.98f - i * 8.f;
		bullet_icon->position.y = WIN_HEIGHT * 0.05f;
		bullet_icon->scale = { 6,36,1 };
		bullet_icon->SetTexture(temp);
		bullet_icons.push_back(bullet_icon);
	}

	atk_level = 0;
	hp_level = 0;

	muzzle[0] = NEW ModelInstance("muzzle1");
	muzzle[1] = NEW ModelInstance("muzzle2");

	muzzle_transform[0] = muzzle[0]->AddTransform();
	muzzle_transform[1] = muzzle[1]->AddTransform();

	muzzle_transform[0]->SetParent(model_transform->GetWorldPtr());
	muzzle_transform[1]->SetParent(model_transform->GetWorldPtr());

	weapon_muzzle_pivot[0] = { 1.f,15.f,-60.f };
	weapon_muzzle_pivot[1] = { 0.284f,16.615f,-66.974f };
	weapon_muzzle_pivot[2] = { -0.522f,15.205f,-83.833f };
	weapon_muzzle_pivot[3] = { -0.341f,11.252f,-104.599f };

	//muzzle_transform[1]->scale = { 0.02f,0.02f ,0.02f };
}

FPSModel::~FPSModel()
{
	SAFE_DELETE(hp_bar);
	SAFE_DELETE(hp_back);
	SAFE_DELETE(model[0]);
	SAFE_DELETE(model[1]);
	SAFE_DELETE(model[2]);
	SAFE_DELETE(model[3]);
	SAFE_DELETE(muzzle[0]);
	SAFE_DELETE(muzzle[1]);
	SAFE_DELETE(move_sound_loop);
	SAFE_DELETE(reload_sound_loop);
	SAFE_DELETE(shotgun_reload_loop);
	SAFE_DELETE(draw_loop);
	SAFE_DELETE(muzzle_loop);
	SAFE_DELETE(idle_loop);
	SAFE_DELETE(shot_loop);

	for (Render2D* bullet_icon : bullet_icons)
	{
		SAFE_DELETE(bullet_icon);
	}
	SAFE_DELETE(bullet_up_icon);
	SAFE_DELETE(atk_up_icon);
	SAFE_DELETE(hp_up_icon);
	//for(Transform* transform : model_transforms)
	//{
	//	SAFE_DELETE(transform);
	//}
	delete collider_transform;
	SAFE_DELETE(obj_collider);
	delete hit_collider;
}

void FPSModel::Init()
{
	collider_transform->position = { 0,6,0 };
	collider_transform->UpdateWorld();
	model_transform->UpdateWorld();
	is_rot_up = true;
	is_move = false;
	is_dash = false;
	is_action = false;

	move_sound_loop->StartLoop();
	draw_loop->StartLoop();
	shot_loop->StartLoop();
	reload_sound_loop->StartLoop();
	atk_level = 0;
	hp_level = 0;
	max_hp = 100;
	money = 0;
	level = 0;
	hp = max_hp;
	ChangeWeapon(Weapon::PISTOL);

	for (UINT i = 0; i < 4; i++)
	{
		Weapon weaponIndex = static_cast<Weapon>(i);
		switch (weaponIndex)
		{
		case PISTOL:
			weapon_bullet_left_num[weaponIndex] = weapon_num_charge[weaponIndex];
			weapon_bullet_num[weaponIndex] = 50;
			break;
		case RIFLE:
			weapon_bullet_left_num[weaponIndex] = weapon_num_charge[weaponIndex];
			weapon_bullet_num[weaponIndex] = 100;
			break;
		case SHOTGUN:
			weapon_bullet_left_num[weaponIndex] = weapon_num_charge[weaponIndex];
			weapon_bullet_num[weaponIndex] = 30;
			break;
		case SNIP_RIFLE:
			weapon_bullet_left_num[weaponIndex] = weapon_num_charge[weaponIndex];
			weapon_bullet_num[weaponIndex] = 10;
			break;
		default:;
		}
	}
}

void FPSModel::Update()
{

	if (!CAMERA->GetIsControl())
	{
		return;
	}

	move_sound_loop->Update();
	shot_loop->Update();
	reload_sound_loop->Update();
	idle_loop->Update();
	draw_loop->Update();
	shotgun_reload_loop->Update();
	muzzle_loop->Update();
	
	if (KEY_DOWN(VK_F3))
	{
		Collider::ChangeDevMode();
	}
	if (KEY_DOWN('1'))
	{
		ChangeWeapon(0);
	}
	if (KEY_DOWN('2'))
	{
		if (PlayScene::stageNum >= 2)
		{
			ChangeWeapon(1);
		}
	}
	if (KEY_DOWN('3'))
	{
		if (PlayScene::stageNum >= 4)
		{
			ChangeWeapon(2);
		}
	}
	if (KEY_DOWN('4'))
	{
		if (PlayScene::stageNum >= 6)
		{
			ChangeWeapon(3);
		}
	}
	if (KEY_DOWN('F'))
	{
		if (money >= 1000 && PlayScene::isClear)
		{
			money -= 1000;
			weapon_bullet_num[0] += 30;
			weapon_bullet_num[1] += 60;
			weapon_bullet_num[2] += 14;
			weapon_bullet_num[3] += 10;
			SOUND->Play("cash");
		}
		else
		{

			SOUND->Play("beep");
		}
	}
	int cost = 200 + 50 * level;
	cost = 0;
	if (KEY_DOWN('G'))
	{
		if (money >= cost && PlayScene::isClear)
		{
			money -= cost;
			level++;
			atk += 0.1f;
			SOUND->Play("cash");
			atk_level++;
		}
		else
		{
			SOUND->Play("beep");
		}
	}
	if (KEY_DOWN('H'))
	{
		if (money >= cost && PlayScene::isClear)
		{
			money -= cost;
			level++;
			max_hp += 20;
			hp = max_hp;
			hp_level++;
			SOUND->Play("cash");
		}
		else
		{
			SOUND->Play("beep");
		}
	}


	if (KEY_DOWN('R'))
	{
		Reload();
	}
	if (KEY_PRESS(VK_LBUTTON))
	{
		Shot();
	}
	
	Move();

	model[cur_weapon_num]->Update();
	model_transform->UpdateWorld();
	collider_transform->UpdateWorld();
	hit_collider->Update();
	obj_collider->Update();


	hp_bar->scale.x = hp / max_hp * hp_back->scale.x;
	hp_bar->Update();
	hp_back->Update();

	hp_up_icon->Update();
	atk_up_icon->Update();
	bullet_up_icon->Update();

	for (Render2D* bullet_icon : bullet_icons)
	{
		bullet_icon->Update();
	}


	if (is_muzzle_ones)
	{
		float randScale = Random(0.2f, 0.3f);
		float randRotZ = Random(0.f, XM_PI);
		muzzle_transform[0]->scale = { randScale,randScale ,randScale };
		muzzle_transform[1]->scale = { randScale,randScale ,randScale };
		muzzle_transform[0]->rotation.z = randRotZ;
		muzzle_transform[1]->rotation.z = randRotZ;
		muzzle_transform[0]->SetScalePivot(0, 0, randScale);
		muzzle_transform[1]->SetScalePivot(0, 0, randScale);
		muzzle_transform[0]->position = weapon_muzzle_pivot[cur_weapon_num];
		muzzle_transform[1]->position = weapon_muzzle_pivot[cur_weapon_num];
		muzzle[0]->Update();
		muzzle[1]->Update();

		LIGHT->data.lights[muzzle_info_num].power = 10.0f;
		LIGHT->data.lights[muzzle_info_num].position = model_transform->position;
		LIGHT->data.lights[muzzle_info_num].position.y += 1;
		LIGHT->data.lights[muzzle_info_num].direction = model_transform->GetForward() * -1;
	}




	LIGHT->data.lights[light_info_num].position = model_transform->position;
	LIGHT->data.lights[light_info_num].position.y += 1;
	Float3 temp = static_cast<Float3>(model_transform->GetForward() * 1.f);
	LIGHT->data.lights[light_info_num].position.x += temp.x;
	LIGHT->data.lights[light_info_num].position.y += temp.y;
	LIGHT->data.lights[light_info_num].position.z += temp.z;
	LIGHT->data.lights[light_info_num].direction = model_transform->GetForward() * -1;

}

void FPSModel::Render()
{
	//hit_collider->Render();
	obj_collider->Render();
	model[cur_weapon_num]->Render();
	
	if (is_muzzle_ones)
	{
		random_muzzle_num = Random(0, 1);
		is_muzzle_ones = false;
	}
	if (is_muzzle)
	{
		if (random_muzzle_num)
		{
			muzzle[0]->Render();
		}
		else
		{
			muzzle[1]->Render();
		}
	}

}

void FPSModel::PostRender()
{
	if (PlayScene::isClear)
	{
		hp_up_icon->Render();
		atk_up_icon->Render();
		bullet_up_icon->Render();
	}

	hp_back->Render();
	hp_bar->Render();

	NumberTexturesRender();

	for (UINT index = 0; index < weapon_bullet_left_num[cur_weapon_num]; index++)
	{
		bullet_icons[index]->Render();
	}
}

void FPSModel::ClearStage()
{
	hp = max_hp;
	money += 1000 + PlayScene::stageNum * 1000;

	weapon_bullet_num[0] += 30;
	weapon_bullet_num[1] += 60;
	weapon_bullet_num[2] += 14;
	weapon_bullet_num[3] += 10;
}

void FPSModel::NumberTexturesRender()
{
	PlayScene::NumberTexturesRender(money,
		{ WIN_WIDTH * 0.9f,WIN_HEIGHT * 0.9f,0 }, { 40.0f,40.0f ,1 });


	PlayScene::NumberTexturesRender(weapon_bullet_num[cur_weapon_num],
		{ WIN_WIDTH * 0.95f,WIN_HEIGHT * 0.1f,0 }, { 40.0f,40.0f ,1 });

	if (PlayScene::isClear)
	{
		int cost = 200 + 50 * level;
		PlayScene::NumberTexturesRender(1000,
			{ CENTER_X - 140,WIN_HEIGHT * 0.02f,0 }, { 30.0f,30.0f ,1 });


		PlayScene::NumberTexturesRender(cost,
			{ CENTER_X,WIN_HEIGHT * 0.02f,0 }, { 30.0f,30.0f ,1 });

		PlayScene::NumberTexturesRender(cost,
			{ CENTER_X + 140,WIN_HEIGHT * 0.02f,0 }, { 30.0f,30.0f ,1 });

		PlayScene::NumberTexturesRender(hp_level,
			{ CENTER_X + 180,WIN_HEIGHT * 0.02f + 70,0 }, { 30.0f,30.0f ,1 });
		PlayScene::NumberTexturesRender(atk_level,
			{ CENTER_X + 40,WIN_HEIGHT * 0.02f + 70,0 }, { 30.0f,30.0f ,1 });
	}

}

void FPSModel::IdleAction()
{
	if (is_action)
		return;

	model[cur_weapon_num]->PlayAni(0, 3);
	idle_loop->limitTime = Random(3.0f, 4.5f);
}

void FPSModel::ChangeWeapon(UINT changeWeaponNum)
{
	if (is_action)
		return;

	cur_weapon_num = static_cast<Weapon>(changeWeaponNum);
	SOUND->Stop("gun_draw");
	SOUND->Play("gun_draw");

	model_transforms[changeWeaponNum]->position = model_transform->position;
	model_transforms[changeWeaponNum]->rotation = model_transform->rotation;
	model_transforms[changeWeaponNum]->scale = model_transform->scale;
	model_transform->position.y = -50;
	model_transform = model_transforms[changeWeaponNum];

	CAMERA->SetTarget(model_transform);
	muzzle_transform[0]->SetParent(model_transform->GetWorldPtr());
	muzzle_transform[1]->SetParent(model_transform->GetWorldPtr());
	//CAMERA->SetHeight(1.0f);
	//CAMERA->SetOffset(Vector3(0.8f, 0, 1.5f));
	model_transform->SetRotPivot(0, 1.0f, 0);

	model[cur_weapon_num]->PlayAni(0, 2);
	draw_loop->StartLoop();

	shot_loop->limitTime = weapon_shot_time[changeWeaponNum];
	reload_sound_loop->limitTime = weapon_reload_time[changeWeaponNum];

	is_action = true;
}

void FPSModel::CollisionCheck()
{
	for (BoxCollider* objCollider : PlayScene::GetColliders())
	{
		objCollider->CollisionPush(this->obj_collider);
	}
}

void FPSModel::SetModel()
{

	
	model[0] = NEW AnimationModel("Assets/Models/idle_pistol/idle_pistol.mesh");
	model[0]->LoadAnimation("Assets/Models/idle_pistol/anims/idle_pistol.anim", 1, true);
	model[0]->LoadAnimation("Assets/Models/attack_pistol/anims/attack_pistol.anim", 1, false);
	model[0]->LoadAnimation("Assets/Models/draw_pistol/anims/draw_pistol.anim", 1, false);
	model[0]->LoadAnimation("Assets/Models/idle_pistol2/anims/idle_pistol2.anim", 1, false);
	model[0]->LoadAnimation("Assets/Models/reload_pistol/anims/reload_pistol.anim", 1, false);
	model[0]->LoadAnimation("Assets/Models/shot_pistol/anims/shot_pistol.anim", 1, false);

	model_transforms.push_back(model[0]->AddInstance());

	model[1] = NEW AnimationModel("Assets/Models/idle_rifle/idle_rifle.mesh");
	model[1]->LoadAnimation("Assets/Models/idle_rifle/anims/idle_rifle.anim", 1, true);
	model[1]->LoadAnimation("Assets/Models/attack_rifle/anims/attack_rifle.anim", 1, false);
	model[1]->LoadAnimation("Assets/Models/draw_rifle/anims/draw_rifle.anim", 1, false);
	model[1]->LoadAnimation("Assets/Models/idle_rifle2/anims/idle_rifle2.anim", 1, false);
	model[1]->LoadAnimation("Assets/Models/reload_rifle/anims/reload_rifle.anim", 1, false);
	model[1]->LoadAnimation("Assets/Models/shot_rifle/anims/shot_rifle.anim", 1, false);
	model_transforms.push_back(model[1]->AddInstance());

	model[2] = NEW AnimationModel("Assets/Models/idle_shotgun/idle_shotgun.mesh");
	model[2]->LoadAnimation("Assets/Models/idle_shotgun/anims/idle_shotgun.anim", 1, true);
	model[2]->LoadAnimation("Assets/Models/attack_shotgun/anims/attack_shotgun.anim", 1, false);
	model[2]->LoadAnimation("Assets/Models/draw_shotgun/anims/draw_shotgun.anim", 1, false);
	model[2]->LoadAnimation("Assets/Models/idle_shotgun2/anims/idle_shotgun2.anim", 1, false);
	model[2]->LoadAnimation("Assets/Models/reload_shotgun/anims/reload_shotgun.anim", 1, false);
	model[2]->LoadAnimation("Assets/Models/shot_shotgun/anims/shot_shotgun.anim", 1, false);
	model_transforms.push_back(model[2]->AddInstance());

	model[3] = NEW AnimationModel("Assets/Models/idle_sniper/idle_sniper.mesh");
	model[3]->LoadAnimation("Assets/Models/idle_sniper/anims/idle_sniper.anim", 1, true);
	model[3]->LoadAnimation("Assets/Models/attack_sniper/anims/attack_sniper.anim", 1, false);
	model[3]->LoadAnimation("Assets/Models/draw_sniper/anims/draw_sniper.anim", 1, false);
	model[3]->LoadAnimation("Assets/Models/idle_sniper2/anims/idle_sniper2.anim", 1, false);
	model[3]->LoadAnimation("Assets/Models/reload_sniper/anims/reload_sniper.anim", 1, false);
	model[3]->LoadAnimation("Assets/Models/shot_sniper/anims/shot_sniper.anim", 1, false);
	model_transforms.push_back(model[3]->AddInstance());
	model_transform = model_transforms[0];

	model[0]->PlayAni(0, 0);
	model[1]->PlayAni(0, 0);
	model[2]->PlayAni(0, 0);
	model[3]->PlayAni(0, 0);

	model[0]->SetSpeed(0, 0.7f);
	model[1]->SetSpeed(0, 0.7f);
	model[2]->SetSpeed(0, 0.7f);
	model[3]->SetSpeed(0, 0.7f);

	model_transforms[0]->scale = { 0.02f,0.02f,0.02f };
	model_transforms[1]->scale = { 0.02f,0.02f,0.02f };
	model_transforms[2]->scale = { 0.02f,0.02f,0.02f };
	model_transforms[3]->scale = { 0.02f,0.02f,0.02f };

	model_transforms[0]->position = { 0.02f,-50.0f,10 };
	model_transforms[1]->position = { 0.02f,-50.0f,10 };
	model_transforms[2]->position = { 0.02f,-50.0f,10 };
	model_transforms[3]->position = { 0.02f,-50.0f,10 };

	model_transforms[0]->UpdateWorld();
	model_transforms[1]->UpdateWorld();
	model_transforms[2]->UpdateWorld();
	model_transforms[3]->UpdateWorld();
}

void FPSModel::Move()
{
	if (ENVIRONMENT->GetIsFreeCam())
	{
		return;
	}
	float padding = 5.0f;
	float cur_move_speed = move_speed;
	//model_transform->rotation.y = XM_PI + model_transform->rotation.y;
	collider_transform->rotation.y = model_transform->rotation.y;
	if (KEY_PRESS(VK_SHIFT))
	{
		is_dash = true;
		cur_move_speed *= 2;
		move_sound_loop->limitTime = 0.3f;
	}

	if (KEY_PRESS('W'))
	{
		is_move = true;
		collider_transform->position -= collider_transform->GetForward() * cur_move_speed * DELTA;
	}
	if (KEY_PRESS('S'))
	{
		is_move = true;
		collider_transform->position += collider_transform->GetForward() * cur_move_speed * DELTA;
	}

	if (KEY_PRESS('A'))
	{
		is_move = true;
		collider_transform->position += collider_transform->GetRight() * cur_move_speed * DELTA;
	}
	if (KEY_PRESS('D'))
	{
		is_move = true;
		collider_transform->position -= collider_transform->GetRight() * cur_move_speed * DELTA;
	}
	CollisionCheck();

	if (is_move)
	{
		if (move_sound_loop->isLimit)
		{
			SOUND->Play("move");
			move_sound_loop->StartLoop();
		}
	}
	if (KEY_UP('W') || KEY_UP('S') || KEY_UP('A') || KEY_UP('D'))
	{
		is_move = false;
		is_dash = false;
	}
	if (KEY_UP(VK_SHIFT))
	{
		is_dash = false;
		move_sound_loop->limitTime = 0.6f;
	}

	if (collider_transform->position.x >= 512.0f - padding)
	{
		collider_transform->position.x = 512.0f - padding;
	}
	if (collider_transform->position.z >= 512.0f - padding)
	{
		collider_transform->position.z = 512.0f - padding;
	}

	if (collider_transform->position.x <= padding)
	{
		collider_transform->position.x = padding;
	}
	if (collider_transform->position.z <= padding)
	{
		collider_transform->position.z = padding;
	}

	model_transform->position = collider_transform->position;
	if (!is_move)
	{
		if (cur_rot > 0.0f)
		{
			cur_rot -= 0.5f * DELTA;
			if (cur_rot < 0.0f)
				cur_rot = 0.0f;
		}
		else
		{
			cur_rot += 0.5f * DELTA;
			if (cur_rot > 0.0f)
				cur_rot = 0.0f;
		}
	}
	else
	{
		if (is_rot_up)
		{
			cur_rot += 0.1f * DELTA;
			if (is_dash)
			{
				cur_rot += 0.1f * DELTA;
			}

			if (cur_rot > max_rot)
			{
				is_rot_up = false;
			}
		}
		else
		{
			cur_rot -= 0.1f * DELTA;
			if (is_dash)
			{
				cur_rot -= 0.1f * DELTA;
			}

			if (cur_rot < min_rot)
			{
				is_rot_up = true;
			}
		}
	}
	model_transform->rotation.x += cur_rot;
	position = collider_transform->position;
}

void FPSModel::CheckActionsLoop()
{
	is_action = false;
}

void FPSModel::ShotGunReloadLoop()
{
	if (weapon_bullet_num[cur_weapon_num] > 0)
	{
		if (reload_sound_loop->isLimit && !is_action)
		{
			if (weapon_bullet_left_num[cur_weapon_num] < weapon_num_charge[cur_weapon_num])
			{
				reload_sound_loop->StartLoop();

				if (cur_weapon_num == Weapon::SHOTGUN)
				{
					SOUND->Stop("gun_reload2");
					SOUND->Play("gun_reload2");
					weapon_bullet_left_num[cur_weapon_num] ++;
					weapon_bullet_num[cur_weapon_num]--;
					is_action = true;
					shotgun_reload_loop->StartLoop();
					model[cur_weapon_num]->PlayAni(0, 4, 10, 10);
					return;
				}
				
			}
		}
	}
	is_action = false;
	model[cur_weapon_num]->PlayAni(0, 0);
}

void FPSModel::Reload()
{
	if (weapon_bullet_num[cur_weapon_num] > 0)
	{
		if (reload_sound_loop->isLimit && !is_action)
		{
			if (weapon_bullet_left_num[cur_weapon_num] < weapon_num_charge[cur_weapon_num])
			{
				reload_sound_loop->StartLoop();

				if (cur_weapon_num == Weapon::SHOTGUN)
				{
					SOUND->Stop("gun_reload2");
					SOUND->Play("gun_reload2");
					weapon_bullet_left_num[cur_weapon_num] ++;
					weapon_bullet_num[cur_weapon_num]--;
					is_action = true;
					shotgun_reload_loop->StartLoop();
					model[cur_weapon_num]->PlayAni(0, 4, 10, 10);
				}
				else
				{
					if (cur_weapon_num == Weapon::SNIP_RIFLE)
					{
						SOUND->Stop("gun_reload3");
						SOUND->Play("gun_reload3");
					}
					else
					{
						SOUND->Stop("gun_reload");
						SOUND->Play("gun_reload");
					}
					int reloadNum = (weapon_num_charge[cur_weapon_num] - weapon_bullet_left_num[cur_weapon_num]);
					int leftBulletNum = weapon_bullet_num[cur_weapon_num];
					if (leftBulletNum - reloadNum > 0)
					{
						weapon_bullet_num[cur_weapon_num] -=
							weapon_num_charge[cur_weapon_num] - weapon_bullet_left_num[cur_weapon_num];
						weapon_bullet_left_num[cur_weapon_num] = weapon_num_charge[cur_weapon_num];
					}
					else
					{
						weapon_bullet_left_num[cur_weapon_num] += weapon_bullet_num[cur_weapon_num];
						weapon_bullet_num[cur_weapon_num] = 0;
					}
					is_action = true;
					model[cur_weapon_num]->PlayAni(0, 4);
				}
			}
		}
	}
}

void FPSModel::Shot()
{
	if (weapon_bullet_left_num[cur_weapon_num] != 0)
	{
		if (shot_loop->isLimit && !is_action)
		{
			muzzle_loop->StartLoop();
			shot_loop->StartLoop();
			
			is_muzzle_ones = true;
			is_muzzle = true;
			const int frame_x = GetSystemMetrics(SM_CXFRAME);
			const int caption_y = GetSystemMetrics(SM_CYCAPTION);
			RECT temp;
			GetWindowRect(hWnd, &temp);
			POINT pt;
			pt.x = CENTER_X + temp.left + frame_x * 0.5f;
			pt.y = CENTER_Y + temp.top + caption_y * 0.5f;
			SetCursorPos(pt.x, pt.y);
			CAMERA->rotation.x -= weapon_rebound[cur_weapon_num];
			weapon_bullet_left_num[cur_weapon_num]--;
			is_atk = true;
			switch (cur_weapon_num)
			{
			case PISTOL:
				SOUND->Stop("gun_shot1");
				SOUND->Play("gun_shot1");
				break;

			case RIFLE:
				SOUND->Stop("gun_shot2");
				SOUND->Play("gun_shot2");
				break;
			case SHOTGUN:
				SOUND->Stop("gun_shot3");
				SOUND->Play("gun_shot3");
				break;
			case SNIP_RIFLE:
				SOUND->Stop("gun_shot4");
				SOUND->Play("gun_shot4");
				break;
			default:;
			}

			model[cur_weapon_num]->PlayAni(0, 5, 5, 5);
			is_action = true;
		}
	}
}

void FPSModel::MuzzleLoop()
{
	is_muzzle = false;
	LIGHT->data.lights[muzzle_info_num].power = 0.0f;
}

