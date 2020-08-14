#pragma once

class AssimpConverter
{
private:
    struct HierarchyNode
    {
        string name;

        XMFLOAT3 translate = { 0.f,0.f,0.f };
        XMFLOAT4 preQuaternion = { 0.f,0.f,0.f,1.f };
        XMFLOAT4 quaternion = { 0.f,0.f,0.f,1.f };
        XMFLOAT3 scale = { 1.f,1.f,1.f };
        XMMATRIX local = XMMatrixIdentity();
        XMMATRIX world = XMMatrixIdentity();
        XMFLOAT4X4 offset;
        int parentID = -1;
        HierarchyNode()
        {
            XMStoreFloat4x4(&offset, XMMatrixIdentity());
        }
    };

    struct VertexType
    {
        XMFLOAT3 position = { 0.f,0.f,0.f };
        XMFLOAT2 uv = { 0.f,0.f };
        XMFLOAT3 normal = { 0.f,0.f,0.f };
        XMFLOAT3 tangent = { 0.f,0.f,0.f };
        XMUINT4 boneid = { 0,0,0,0 };
        XMFLOAT4 weight = { 0.f,0.f,0.f,0.f };
    };

    struct Mesh
    {
        UINT ID = 0;
        UINT matrialID = 0;
        vector<VertexType> vertices;
        vector<UINT> indices;
    };

    struct Material
    {
        string name;
        UINT ID = 0;

        XMFLOAT3 diffuse = { 0.f,0.f,0.f };
        XMFLOAT3 ambient = { 0.f,0.f,0.f };
        XMFLOAT3 specular = { 0.f,0.f,0.f };
        XMFLOAT3 emissive = { 0.f,0.f,0.f };
        XMFLOAT3 tranparent = { 1.f,1.f,1.f };
        XMFLOAT3 reflective = { 0.f,0.f,0.f };

        FLOAT opacity = 0.f;
        FLOAT transparentfactor = 0.f;
        FLOAT bumpscaling = 0.f;
        FLOAT shininess = 0.f;
        FLOAT reflectivity = 0.f;
        FLOAT shininessstrength = 0.f;
        FLOAT refracti = 0.f;

        string diffusefile;
        string specularfile;
        string ambientfile;
        string emissivefile;
        string heightfile;
        string normalfile;
        string shininessfile;
        string opacityfile;
        string displacementfile;
        string lightMapfile;
        string reflectionfile;
        string basecolorfile;
        string normalcamerafile;
        string emissioncolorfile;
        string metalnessfile;
        string diffuseroughnessfile;
        string ambientocculsionfile;
    };

    struct Key
    {
        XMFLOAT3 translate = { 0.f,0.f,0.f };
        float keyType=0.f;
        XMFLOAT4 quaternion = { 0.f,0.f,0.f,1.f };
        XMFLOAT3 scale = { 1.f,1.f,1.f };
    };

    struct Animation
    {
        string name;

        float tickPerSec = 0.f;
        float duration = 0.f;

        vector<vector<Key>> keyframes;//노드마다 전체 키프레임별 키값을 찍어버리자.어차피 컴퓨트셰이더에서 메트릭스로 키프레임마다 변환 시킬꺼라 프레임은 정확히 떨어지고 소숫점으로 러프시키는게 나음.
        //바깥쪽은 프레임 카운트의 전체 키프레임, 안쪽은 전체 하이어라키노드 실제 웨이트 본만 계산후 메트릭스 넘기는 것보다는 시간을 걸릴지 몰라도 slerp를 안 쓸순 없다!
    };
public:
    AssimpConverter();
    ~AssimpConverter();

    void ConvertMesh(const string path, const string outPath = "");
    void LoadAnimation(const string path);
    void SaveAnimation(string path = "", UINT takeNum = 0);
private:
    void LoadMesh();
    void SaveMesh();//하이어라키노드와 메쉬정보, 메터리얼정보, 본정보 저장

    void ResetMeshs();

    void NodeHeirarchy(const aiNode* node, int value);
    void InitBones(const aiScene* scene);
    void InitMesh(const aiScene* scene);
    void BindSkin(const aiMesh* mesh, vector<VertexType>& vertices);

    UINT InitMaterials(const aiScene* scene, UINT index);
    string SaveTexture(const aiScene* scene, string file);

    void ResetAnimations();

    void AddAnimation(const aiScene* scene);
    void CalcInterpolatedPosition(const aiNodeAnim* nodeAnim, const UINT frame, XMFLOAT3& trans);
    void CalcInterpolatedQuaternion(const aiNodeAnim* nodeAnim, const UINT frame, XMFLOAT4& quat);
    void CalcInterpolatedScaling(const aiNodeAnim* nodeAnim, const UINT frame, XMFLOAT3& scale);


    //map보다 vector속도가 빠를것 같아 vector만 사용 vector의 서칭은 find_if(nodeList.begin(), nodeList.end(), [bName](const NodeInfo* a)->bool { return a->name == bName; });
    //map과 속도비교해봐야 함. 대략 평균 200개에서 300개 조인트 생각하고 속도 비교해봐야함 최대치 600개정도(512개)
    //(ex: 드래곤네스트 드래곤 338개, 뮤오리진 엠퍼사이저 250개, 레이븐 에일라 423개, 동 에인투비스 519개)
    //Vector보다 map이 더 빠름 ㅋ;;;
    vector<HierarchyNode> hierarchyNodes;//모든 애니메이션 키적용은 하이어라키상에서 이루어짐
    map<string, UINT> hierarchyMap;
    //vector<UINT> bones;//모델이 가진 모든 메쉬에 영향을 주는 조인트의 모음. 버텍스셰이더에서 계산될 메트릭스 혹은 SRT의 최소 모음
    map<UINT, UINT> meshMap;
    vector<Mesh> meshs;//랜더링에 사용되는 실제 메쉬데이터. 복수의 메터리얼을 적용한 메쉬는 분리되어 메터리얼마다 하나씩의 메쉬를 가짐. 같은 메터리얼을 가진 메쉬가 있는 경우도 있음
    vector<Material> materials;//씬안의 메쉬들이 사용하는 메터리얼 모음, 메쉬가 있는 메터리얼만 생성
    string name = "";
    string animName = "";
    string meshPath = "";
    string outPath = "";
    vector<Animation> animations;
};

