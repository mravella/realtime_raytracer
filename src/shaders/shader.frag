#version 330


#define M_PI 3.14159265359
#define M_INF 1e16
#define NUM_OBJS 1
#define NUM_LIGHTS 1
#define BLACK vec4(0.0, 0.0, 0.0, 1.0);
#define kA 0.5
#define kD 0.5
#define kS 0.5

out vec4 outColor;
uniform float width;
uniform float height;
uniform mat4 filmToWorld;
uniform mat4 inverseView;
uniform vec3 eye;

float hit = 0.0;

struct lighting
{
    vec3 diffuse;
    vec3 spec;
};

struct obj
{
    vec3 ca;
    vec3 cd;
    vec3 cs;
    mat4 xform;
    int type;
    float shininess;
} objs[NUM_OBJS];

struct light
{
    vec3 color;
    vec3 function;
    vec3 pos;
} lights[NUM_LIGHTS];

struct isect
{
    float t;
    obj obj;
} i;

float smallestPos(float a, float b)
{
    if (a <= 0.0 && b < 0.0) {
        return -1.0;
    }
    else if (a <= 0.0 && b > 0.0) {
        return b;
    }
    else if (a > 0.0 && b <= 0.0) {
        return a;
    }
    else {
        return min(a, b);
    }
}

float quadratic(float a, float b, float c)
{
    float dscr = pow(b, 2.0) - (4.0 * a * c);

    if (dscr < 0.0) {
        return -1.0;
    }
    else {
        return smallestPos((-b - pow(dscr, 0.5)) / (2.0 * a), (-b + pow(dscr, 0.5)) / (2.0 * a));
    }
}

float intersectSphere(vec3 ro, vec3 rd)
{
    float a = pow(rd.x, 2.0) + pow(rd.y, 2.0) + pow(rd.z, 2.0);
    float b = 2.0 * (rd.x * ro.x + rd.y * ro.y + rd.z * ro.z);
    float c = pow(ro.x, 2.0) + pow(ro.y, 2.0) + pow(ro.z, 2.0) - 0.25;

    return quadratic(a, b, c);
}

vec3 getSphereNormal(vec3 pos)
{
    return normalize(pos);
}

isect intersectObjs(vec3 ro, vec3 rd)
{
    isect i;
    i.t = -1.0;
    float minT = -1.0;
    obj minObj;
    float currT;
    for (int i = 0; i < NUM_OBJS; i++)
    {
        vec3 p = vec3(inverse(objs[i].xform) * vec4(ro, 1.0));
        vec3 d = vec3(inverse(objs[i].xform) * vec4(rd, 0.0));

        currT = intersectSphere(p, d);
        if ((currT < minT && currT != -1.0) || (currT > minT && minT == -1.0))
        {
            minT = currT;
            minObj = objs[i];
        }
    }
    i.t = minT;
    i.obj = minObj;
    return i;
}

lighting computeLighting(vec3 pos, vec3 norm, vec3 rd, float shininess)
{
    vec3 diffuse = vec3(0.0, 0.0, 0.0);
    vec3 spec = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        vec3 vertToLight = lights[i].pos - pos;
        vec3 lightDir = normalize(vertToLight);
        float dist = sqrt(vertToLight.x * vertToLight.x + vertToLight.y * vertToLight.y + vertToLight.z * vertToLight.z);

        vec3 reflectionVec = normalize(reflect(-lightDir, norm));
        vec3 posAug = pos + (norm / 1000.0);



        float falloff = max(1.0, (lights[i].function.x + dist * lights[i].function.y + dist * dist * lights[i].function.z));
        spec.r += min(1.0, pow(max(0.0, dot(reflectionVec, rd)), shininess) * lights[i].color.r) / falloff;
        spec.g += min(1.0, pow(max(0.0, dot(reflectionVec, rd)), shininess) * lights[i].color.g) / falloff;
        spec.b += min(1.0, pow(max(0.0, dot(reflectionVec, rd)), shininess) * lights[i].color.b) / falloff;
        diffuse.r += max(0.0, dot(norm, lightDir) * lights[i].color.r) / falloff;
        diffuse.g += max(0.0, dot(norm, lightDir) * lights[i].color.g) / falloff;
        diffuse.b += max(0.0, dot(norm, lightDir) * lights[i].color.b) / falloff;

    }
    return lighting(diffuse, spec);
}

void init()
{
//    objs[0].ca = vec3(0.3, 0.0, 0.4);
//    objs[0].cd = vec3(0.3, 0.0, 0.4);
//    objs[0].cs = vec3(0.3, 0.0, 0.4);
//    objs[0].xform = mat4(1.2, 0.0, 0.0, 0.0,
//                         0.0, 1.2, 0.0, 0.0,
//                         0.0, 0.0, 1.2, 0.0,
//                         0.0, 0.0, 0.0, 1.0);
//    objs[0].type = 1;
//    objs[0].shininess = 30.0;

    objs[0].ca = vec3(0.0, 0.0, 0.0);
    objs[0].cd = vec3(0.0, 0.0, 1.0);
    objs[0].cs = vec3(1.0, 1.0, 1.0);
    objs[0].xform = mat4(0.25, 0.0, 0.0, 0.0,
                         0.0, 0.25, 0.0, 0.0,
                         0.0, 0.0, 0.25, 0.0,
                         0.0, 0.0, 0.0, 1.0);
    objs[0].type = 1;
    objs[0].shininess = 25.0;


//    lights[0].color = vec3(1.0, 1.0, 1.0);
//    lights[0].function = vec3(0.5, 0.0, 0.0);
//    lights[0].pos = vec3(3.0, 3.0, -3.0);

    lights[0].color = vec3(1.0, 1.0, 1.0);
    lights[0].function = vec3(0.0, 0.0, 0.0);
    lights[0].pos = vec3(3.0, 3.0, -3.0);

//    lights[1].color = vec3(1.0, 1.0, 1.0);
//    lights[1].function = vec3(1.5, 0.0, 0.0);
//    lights[1].pos = vec3(-1.0, 1.8, -2.0);
}



void main(void)
{
    vec2 uv = (-1.0 + 2.0*gl_FragCoord.xy / vec2(width, height)) * vec2(width/height, 1.0);
    vec3 ro = vec3(0.0, 0.0, 1.0);
    vec3 rd = normalize(vec3(uv, -1.0));

//    vec4 film = vec4(((2.0 * gl_FragCoord.x) / width) - 1.0, 1.0 - ((2.0 * gl_FragCoord.y) / height), -1.0, 1.0);
//    vec4 world = filmToWorld * film;
//    vec3 rd = normalize(vec3(world) - eye);
//    vec3 ro = eye;

    init();
    isect i = intersectObjs(ro, rd);

    if (i.t == -1.0)  {
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
                return;
    }

    vec3 worldPos = ro * i.t + rd;

    vec3 norm = getSphereNormal(worldPos);

    vec3 ambient = i.obj.ca * kA;

    lighting l = computeLighting(worldPos, norm, rd, i.obj.shininess);
    vec3 diffuse = l.diffuse;
    vec3 spec = l.spec;


    diffuse *= kD * i.obj.cd;
    spec *= kS * i.obj.cs;

    outColor = vec4(ambient + diffuse + spec, 1.0);
}

