#pragma once
enum Weapon
{
	PISTOL,
	RIFLE,
	SHOTGUN,
	SNIP_RIFLE
};



class FPSModel
{
public:
	FPSModel();
	~FPSModel();

	void Init();
	void Update();
	void Render();
	void PostRender();
	void ClearStage();
	
	Transform*		GetModelTransform()		const { return model_transform; }
	Transform*		GetColliderTransform()	const { return collider_transform; }
	AnimationModel* GetModel()				const { return model[cur_weapon_num]; }
	bool			GetIsAttack()			const { return is_atk; }
	float			GetAtk()				const { return atk * weapon_atk[cur_weapon_num]; }
	void			SetIsAttack(bool is_atk) { this->is_atk = is_atk; }
private:
	void NumberTexturesRender();
	void IdleAction();
	void ChangeWeapon(UINT changeWeaponNum);
	void CollisionCheck();
	void SetModel();
	void Move();
	void CheckActionsLoop();
	void ShotGunReloadLoop();
	void Reload();
	void Shot();
	void MuzzleLoop();
public:
	static float		money;
	static float		hp;
	static Vector3		position;
private:
	AnimationModel*		model[4];
	ModelInstance*		muzzle[2];
	Loop*				move_sound_loop;
	Loop*				shot_loop;
	Loop*				draw_loop;
	Loop*				reload_sound_loop;
	Loop*				shotgun_reload_loop;
	Loop*				idle_loop;
	Loop*				muzzle_loop;
	float				move_speed;
	float				max_rot;
	float				min_rot;
	float				cur_rot;

	float				weapon_rebound[4];
	float				weapon_atk[4];
	UINT				weapon_bullet_left_num[4];
	Vector3				weapon_muzzle_pivot[4];
	UINT				weapon_bullet_num[4];
	UINT				weapon_num_charge[4];
	UINT				atk_level;
	UINT				hp_level;
	UINT				level;
	float				weapon_reload_time[4];
	float				weapon_shot_time[4];
	Weapon				cur_weapon_num;

	
	UINT				bullet_left_num;				
	float				max_hp;
	float				atk;
	int					light_info_num;
	int					muzzle_info_num;
	int					random_muzzle_num;
	bool				is_rot_up;
	bool				is_move;
	bool				is_dash;
	bool				is_action;
	bool				is_atk;
	bool				is_muzzle;
	bool				is_muzzle_ones;

	Transform*			model_transform;
	Transform*			muzzle_transform[2];
	Transform*			collider_transform;
	vector<Transform*> model_transforms;
	
	BoxCollider*		hit_collider;
	SphereCollider*		obj_collider;
	
	Render2D*			hp_bar;
	Render2D*			hp_back;
	Render2D*			gun_icon;
	Render2D*			hp_up_icon;
	Render2D*			atk_up_icon;
	Render2D*			bullet_up_icon;
	vector<Render2D*>	bullet_icons;
	
};
