using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Wander : MonoBehaviour
{

    public Rigidbody rb;

    private Vector3 desired_velocity;
    private Vector3 random_veloccity;
    private Vector3 steering;

    private GameObject target;

    private int max_velocity = 5;

    // Use this for initialization
    void Start()
    {
        rb = GetComponent<Rigidbody>();

        target = GameObject.Find("Target");

        rb.velocity = new Vector3(0, 0, 0);
    }

    // Update is called once per frame
    void FixedUpdate()
    {
        desired_velocity = Vector3.Normalize(target.transform.position - rb.position) * max_velocity;

        // Random Movement every frame
        random_veloccity = Random.onUnitSphere * 3;
        desired_velocity.x += random_veloccity.x;
        desired_velocity.z += random_veloccity.z;

        steering = desired_velocity - rb.velocity;

        rb.velocity += steering / rb.mass;
    }

}
