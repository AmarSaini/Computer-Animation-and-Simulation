using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Evade : MonoBehaviour
{

    public Rigidbody rb;

    private Vector3 desired_velocity;
    private Vector3 steering;

    private GameObject target;
    private Vector3 estimatedNextPosition;
    private float timeToLookAhead = 1.0f;

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

        estimatedNextPosition = target.transform.position + target.GetComponent<Rigidbody>().velocity * timeToLookAhead;

        desired_velocity = Vector3.Normalize(estimatedNextPosition - rb.position) * max_velocity;
        steering = -desired_velocity - rb.velocity;

        rb.velocity += steering / rb.mass;

    }

}
