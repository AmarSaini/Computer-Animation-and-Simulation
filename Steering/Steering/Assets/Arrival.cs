using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Arrival : MonoBehaviour
{

    public Rigidbody rb;

    private Vector3 desired_velocity;
    private Vector3 steering;

    private GameObject target;

    private int max_velocity = 5;
    private float arrivalRadius = 2;

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
        print(Vector3.Distance(target.transform.position, rb.position));
        // Check if arriving close to target
        if (Vector3.Distance(target.transform.position, rb.position) - 1 < arrivalRadius)
        {
            desired_velocity *= (Vector3.Distance(target.transform.position, rb.position) - 1) / arrivalRadius;
        }

        steering = desired_velocity - rb.velocity;

        rb.velocity += steering / rb.mass;
    }

}
